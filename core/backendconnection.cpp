#include "backendconnection.h"
#include "dialogjsonreader.h"
#include "dialogjsonwriter.h"
#include "logger.h"

namespace Core
{

namespace
{

int generateQueryId()
{
	static int s_queryId = 0;
	return ++s_queryId;
}

QJsonObject toJson(const Dialog& dialog)
{
	return DialogJsonWriter().writeToObject(dialog);
}

QByteArray toString(const QJsonObject& object)
{
	return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

QJsonObject toJson(const User& user)
{
	return QJsonObject{ { "username", user.name }, { "password", user.name }, { "admin", user.admin } };
}

}

BackendConnection::BackendConnection(const QUrl& url)
	: m_webSocket(url)
{
	connect(&m_webSocket, &WebSocket::disconnected, this, &BackendConnection::onWebSocketDisconnected);
	connect(&m_webSocket, &WebSocket::messageReceived, this, &BackendConnection::onWebSocketMessage);
	connect(&m_webSocket, &WebSocket::error, this, &BackendConnection::onWebSocketError);
}

BackendConnection::~BackendConnection()
{
	disconnect(&m_webSocket, 0, this, 0);
}

IBackendConnection::QueryId BackendConnection::logIn(const QString& username, const QString& password)
{
	const QJsonObject message = {
		{ "queryId", generateQueryId() },
		{ "type", "log_in" },
		{ "username", username },
		{ "password", password },
		{ "role", 1 }
	};

	return sendMessage(message);
}

IBackendConnection::QueryId BackendConnection::logOut()
{
	const QJsonObject message = {
		{ "queryId", generateQueryId() },
		{ "type", "log_out" }
	};

	return sendMessage(message);
}

IBackendConnection::QueryId BackendConnection::loadDialogs()
{
	const QJsonObject message = {
		{ "queryId", generateQueryId() },
		{ "type", "dialogs_load" }
	};

	return sendMessage(message);
}

IBackendConnection::QueryId BackendConnection::updateDialogs(const Update<Dialog>& update)
{
	QJsonArray updatedDialogs;
	for (const auto& originalDialog : update.updated.keys())
	{
		const QJsonObject dialogObject = {
			{ "name", originalDialog.name },
			{ "difficulty", static_cast<int>(originalDialog.difficulty) },
			{ "value", toJson(update.updated.value(originalDialog)) }
		};
		updatedDialogs << dialogObject;
	}

	QJsonArray deletedDialogs;
	for (const auto& dialog : update.deleted)
	{
		deletedDialogs << QJsonObject{
			{ "name", dialog.name },
			{ "difficulty", static_cast<int>(dialog.difficulty) }
		};
	}

	QJsonArray addedDialogs;
	for (const auto& dialog : update.added)
	{
		addedDialogs << QJsonObject{ { "value", toJson(dialog) } };
	}

	QJsonObject updateObject;
	if (!updatedDialogs.empty())
	{
		updateObject["updated"] = updatedDialogs;
	}
	if (!addedDialogs.empty())
	{
		updateObject["added"] = addedDialogs;
	}
	if (!deletedDialogs.empty())
	{
		updateObject["deleted"] = deletedDialogs;
	}

	QJsonObject message = {
		{ "queryId", generateQueryId() },
		{ "type", "dialogs_update" },
		{ "update", updateObject }
	};

	return sendMessage(message);
}

IBackendConnection::QueryId BackendConnection::loadUsers()
{
	const QJsonObject message = {
		{ "queryId", generateQueryId() },
		{ "type", "users_load" }
	};

	return sendMessage(message);
}

IBackendConnection::QueryId BackendConnection::updateUsers(const Update<User>& update)
{
	QJsonArray updatedUsers;
	for (const auto& originalUser : update.updated.keys())
	{
		const QJsonObject updatedUserObject = {
			{ "username", originalUser.name },
			{ "value", toJson(update.updated.value(originalUser)) }
		};
		updatedUsers << updatedUserObject;
	}

	QJsonArray deletedUsers;
	for (const auto& user : update.deleted)
	{
		deletedUsers << QJsonObject{ { "username", user.name } };
	}

	QJsonArray addedUsers;
	for (const auto& user : update.added)
	{
		addedUsers << QJsonObject{ { "value", toJson(user) } };
	}

	QJsonObject updateObject;
	if (!updatedUsers.empty())
	{
		updateObject["updated"] = updatedUsers;
	}
	if (!addedUsers.empty())
	{
		updateObject["added"] = addedUsers;
	}
	if (!deletedUsers.empty())
	{
		updateObject["deleted"] = deletedUsers;
	}

	QJsonObject message = {
		{ "queryId", generateQueryId() },
		{ "type", "users_update" },
		{ "update", updateObject }
	};

	return sendMessage(message);
}

void BackendConnection::onWebSocketDisconnected()
{
	while (!m_activeQueries.empty())
	{
		const auto queryId = m_activeQueries.firstKey();
		auto processor = m_activeQueries.take(queryId);
		processor.processWebSocketDisconnect(queryId, "Соединение с сервером было закрыто.");
	}
}

void BackendConnection::onWebSocketMessage(const QJsonObject& message)
{
	const QString type = message["type"].toString();
	LOG << "Received message" << ARG(type);

	const IBackendConnection::QueryId queryId = message["queryId"].toInt();

	const auto processor = m_activeQueries.take(queryId);

	const QJsonObject payload = message["payload"].toObject();
	if (!payload.contains("error"))
	{
		processor.processData(queryId, payload["data"].toObject());
	}
	else
	{
		processor.processError(queryId, payload["error"].toObject());
	}

	LOG << ARG(queryId) << " pop from active, " << m_activeQueries.size() << " active queries left";
}

void BackendConnection::onWebSocketError(const QString& errorMessage)
{
	while (!m_activeQueries.empty())
	{
		const auto queryId = m_activeQueries.firstKey();
		auto processor = m_activeQueries.take(queryId);
		processor.processWebSocketError(queryId, "Соединение с сервером было разорвано: " + errorMessage);
	}
}

IBackendConnection::QueryId BackendConnection::sendMessage(const QJsonObject& message)
{
	const IBackendConnection::QueryId queryId = message["queryId"].toInt();

	m_activeQueries.insert(queryId, makeProcessor(message["type"].toString()));

	LOG << ARG(queryId) << " pushed to active";

	return m_webSocket.sendMessage(message);
}

void BackendConnection::onLogInSuccess(IBackendConnection::QueryId queryId)
{
	emit loggedIn(queryId);
}

void BackendConnection::onLogInFailure(IBackendConnection::QueryId queryId, const QJsonObject& message)
{
	if (!message.contains("error") || message["error"].type() != QJsonValue::String)
	{
		LOG << "Message" << ARG2(message["type"], "type") << " must have \"error\" string property";
		return;
	}

	const QString error = message["error"].toString();
	LOG << "Message" << ARG2(message["type"], "type") << ARG(error);

	emit logInFailed(queryId, error);
}

void BackendConnection::onDialogsLoadSuccess(IBackendConnection::QueryId queryId, const QJsonObject& message)
{
	if (!message.contains("dialogs") || message["dialogs"].type() != QJsonValue::Array)
	{
		LOG << "Message" << ARG2(message["type"], "type") << " must have \"dialogs\" array property";
		return;
	}

	const QJsonArray dialogsArray = message["dialogs"].toArray();
	QList<Dialog> result;
	for (int i = 0; i < dialogsArray.size(); ++i)
	{
		const QJsonValue& dialogValue = dialogsArray[i];
		if (!dialogValue.isObject())
		{
			LOG << "Faled to parse dialog #" << i << " - value type must be an object (actual type is " << dialogValue.type() << ")";
			continue;
		}

		const QJsonObject dialogObject = dialogValue.toObject();

		bool ok = false;
		Dialog dialog = DialogJsonReader().read(toString(dialogObject), ok);
		if (!ok)
		{
			LOG << "Failed to parse dialog #" << i;
			continue;
		}

		result << dialog;
	}

	emit dialogsLoaded(queryId, result);
}

void BackendConnection::onDialogsLoadFailure(IBackendConnection::QueryId queryId, const QJsonObject& message)
{
	if (!message.contains("error") || message["error"].type() != QJsonValue::String)
	{
		LOG << "Message" << ARG2(message["type"], "type") << " must have \"error\" string property";
		return;
	}

	const QString error = message["error"].toString();
	LOG << "Message" << ARG2(message["type"], "type") << ARG(error);
	emit dialogsLoadFailed(queryId, error);
}

void BackendConnection::onDialogsUpdateSuccess(IBackendConnection::QueryId queryId)
{
	emit dialogsUpdated(queryId);
}

void BackendConnection::onDialogsUpdateFailure(IBackendConnection::QueryId queryId, const QJsonObject& message)
{
	if (!message.contains("error") || message["error"].type() != QJsonValue::String)
	{
		LOG << "Message" << ARG2(message["type"], "type") << " must have \"error\" string property";
		return;
	}

	const QString error = message["error"].toString();
	LOG << "Message" << ARG2(message["type"], "type") << ARG(error);
	emit dialogsUpdateFailed(queryId, error);
}

void BackendConnection::onUsersLoadSuccess(IBackendConnection::QueryId queryId, const QJsonObject& message)
{
	if (!message.contains("users") || message["users"].type() != QJsonValue::Array)
	{
		LOG << "Message" << ARG2(message["type"], "type") << " must have \"users\" array property";
		return;
	}

	const QJsonArray usersArray = message["users"].toArray();
	QList<User> result;
	for (int i = 0; i < usersArray.size(); ++i)
	{
		const QJsonValue& userValue = usersArray[i];
		if (!userValue.isObject())
		{
			LOG << "Faled to parse user #" << i << " - value type must be an object (actual type is " << userValue.type() << ")";
			continue;
		}

		const QJsonObject userObject = userValue.toObject();

		if (!userObject.contains("Username") || !userObject["Username"].isString())
		{
			LOG << "Faled to parse user #" << i << " - object must have \"Username\" string property";
			continue;
		}

		if (!userObject.contains("Admin") || !userObject["Admin"].isBool())
		{
			LOG << "Faled to parse user #" << i << " - object must have \"Admin\" boolean property";
			continue;
		}

		result << User(userObject["Username"].toString(), userObject["Admin"].toBool());
	}

	emit usersLoaded(queryId, result);
}

void BackendConnection::onUsersLoadFailure(IBackendConnection::QueryId queryId, const QJsonObject& message)
{
	if (!message.contains("error") || message["error"].type() != QJsonValue::String)
	{
		LOG << "Message" << ARG2(message["type"], "type") << " must have \error\" string property";
		return;
	}

	const QString error = message["error"].toString();
	LOG << "Message" << ARG2(message["type"], "type") << ARG(error);
	emit usersLoadFailed(queryId, error);
}

void BackendConnection::onUsersUpdateSuccess(IBackendConnection::QueryId queryId)
{
	emit usersUpdated(queryId);
}

void BackendConnection::onUsersUpdateFailure(IBackendConnection::QueryId queryId, const QJsonObject& message)
{
	if (!message.contains("error") || message["error"].type() != QJsonValue::String)
	{
		LOG << "Message" << ARG2(message["type"], "type") << " must have \error\" string property";
		return;
	}

	const QString error = message["error"].toString();
	LOG << "Message" << ARG2(message["type"], "type") << ARG(error);
	emit usersUpdateFailed(queryId, error);
}

BackendConnection::Processor BackendConnection::makeProcessor(const QString& queryType)
{
	if (queryType == "log_in")
	{
		return Processor(
			[this](IBackendConnection::QueryId queryId, const QJsonObject&) { onLogInSuccess(queryId); },
			[this](IBackendConnection::QueryId queryId, const QJsonObject& error) { onLogInFailure(queryId, error); },
			[this](IBackendConnection::QueryId queryId, const QString& errorMessage) { emit logInFailed(queryId, errorMessage); },
			[this](IBackendConnection::QueryId queryId, const QString& errorMessage) { emit logInFailed(queryId, errorMessage); }
		);
	}

	if (queryType == "dialogs_load")
	{
		return Processor(
			[this](IBackendConnection::QueryId queryId, const QJsonObject& data) { onDialogsLoadSuccess(queryId, data); },
			[this](IBackendConnection::QueryId queryId, const QJsonObject& error) { onDialogsLoadFailure(queryId, error); },
			[this](IBackendConnection::QueryId queryId, const QString& errorMessage) { emit dialogsLoadFailed(queryId, errorMessage); },
			[this](IBackendConnection::QueryId queryId, const QString& errorMessage) { emit dialogsLoadFailed(queryId, errorMessage); }
		);
	}

	if (queryType == "dialogs_update")
	{
		return Processor(
			[this](IBackendConnection::QueryId queryId, const QJsonObject&) { onDialogsUpdateSuccess(queryId); },
			[this](IBackendConnection::QueryId queryId, const QJsonObject& error) { onDialogsUpdateFailure(queryId, error); },
			[this](IBackendConnection::QueryId queryId, const QString& errorMessage) { emit dialogsUpdateFailed(queryId, errorMessage); },
			[this](IBackendConnection::QueryId queryId, const QString& errorMessage) { emit dialogsUpdateFailed(queryId, errorMessage); }
		);
	}
	if (queryType == "users_load")
	{
		return Processor(
			[this](IBackendConnection::QueryId queryId, const QJsonObject& data) { onUsersLoadSuccess(queryId, data); },
			[this](IBackendConnection::QueryId queryId, const QJsonObject& error) { onUsersLoadFailure(queryId, error); },
			[this](IBackendConnection::QueryId queryId, const QString& errorMessage) { emit usersLoadFailed(queryId, errorMessage); },
			[this](IBackendConnection::QueryId queryId, const QString& errorMessage) { emit usersLoadFailed(queryId, errorMessage); }
		);
	}

	if (queryType ==  "users_update")
	{
		return Processor(
			[this](IBackendConnection::QueryId queryId, const QJsonObject&) { onUsersUpdateSuccess(queryId); },
			[this](IBackendConnection::QueryId queryId, const QJsonObject& error) { onUsersUpdateFailure(queryId, error); },
			[this](IBackendConnection::QueryId queryId, const QString& errorMessage) { emit usersUpdateFailed(queryId, errorMessage); },
			[this](IBackendConnection::QueryId queryId, const QString& errorMessage) { emit usersUpdateFailed(queryId, errorMessage); }
		);
	}

	return Processor();
}

}
