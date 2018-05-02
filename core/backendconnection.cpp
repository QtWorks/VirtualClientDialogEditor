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

// TODO: handle server disconnects - interrupt current queries, display some notifications in interface?
BackendConnection::BackendConnection(const QUrl& url)
	: m_webSocket(url)
	, m_connected(false)
{
	connect(&m_webSocket, &WebSocket::connected, this, &BackendConnection::onWebSocketConnected);
	connect(&m_webSocket, &WebSocket::disconnected, this, &BackendConnection::onWebSocketDisconnected);
	connect(&m_webSocket, &WebSocket::messageReceived, this, &BackendConnection::onWebSocketMessage);

	m_messageProcessors = {
		{ "log_in_success", [this](const QJsonObject& message){ onLogInSuccess(message); } },
		{ "log_in_failure", [this](const QJsonObject& message){ onLogInFailure(message); } },
		//{ "log_out_success", [this](const QJsonObject& message){ onLogOutSuccess(message); } },
		//{ "log_out_failure", [this](const QJsonObject& message){ onLogOutFailure(message); } },
		{ "dialogs_load_success", [this](const QJsonObject& message){ onDialogsLoadSuccess(message); } },
		{ "dialogs_load_failure", [this](const QJsonObject& message){ onDialogsLoadFailure(message); } },
		{ "dialogs_update_success", [this](const QJsonObject& message){ onDialogsUpdateSuccess(message); } },
		{ "dialogs_update_failure", [this](const QJsonObject& message){ onDialogsUpdateFailure(message); } },
		{ "users_load_success", [this](const QJsonObject& message){ onUsersLoadSuccess(message); } },
		{ "users_load_failure", [this](const QJsonObject& message){ onUsersLoadFailure(message); } },
		{ "users_update_success", [this](const QJsonObject& message){ onUsersUpdateSuccess(message); } },
		{ "users_update_failure", [this](const QJsonObject& message){ onUsersUpdateFailure(message); } }
	};
}

void BackendConnection::logIn(const QString& username, const QString& password)
{
	const QJsonObject message = {
		{ "queryId", generateQueryId() },
		{ "type", "log_in" },
		{ "username", username },
		{ "password", password },
		{ "role", 1 }
	};

	sendMessage(message);
}

void BackendConnection::logOut()
{
	const QJsonObject message = {
		{ "queryId", generateQueryId() },
		{ "type", "log_out" }
	};

	sendMessage(message);
}

void BackendConnection::loadDialogs()
{
	const QJsonObject message = {
		{ "queryId", generateQueryId() },
		{ "type", "dialogs_load" }
	};

	sendMessage(message);
}

void BackendConnection::updateDialogs(const Update<Dialog>& update)
{
	QJsonArray updatedDialogs;
	for (const auto& dialog : update.updated)
	{
		const QJsonObject dialogObject = {
			{ "name", dialog.name },
			{ "difficulty", static_cast<int>(dialog.difficulty) },
			{ "value", toJson(update.updated[dialog]) }
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

	sendMessage(message);
}

void BackendConnection::loadUsers()
{
	const QJsonObject message = {
		{ "queryId", generateQueryId() },
		{ "type", "users_load" }
	};

	sendMessage(message);
}

void BackendConnection::updateUsers(const Update<User>& update)
{
	QJsonArray updatedUsers;
	for (const auto& user : update.updated)
	{
		const QJsonObject updatedUserObject = {
			{ "username", user.name },
			{ "value", toJson(update.updated[user]) }
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

	sendMessage(message);
}

void BackendConnection::onWebSocketConnected()
{
	m_connected = true;

	while (!m_pendingMessages.isEmpty())
	{
		auto message = m_pendingMessages.takeFirst();
		sendMessage(message);
	}
}

void BackendConnection::onWebSocketDisconnected()
{
	m_connected = false;
}

void BackendConnection::onWebSocketMessage(const QJsonObject& message)
{
	const QString type = message["type"].toString();
	LOG << "Received message" << ARG(type);

	if (m_messageProcessors.contains(type))
	{
		const auto processor = m_messageProcessors[type];
		processor(message);
	}
	else
	{
		LOG << "Unhandled message: " << type;
	}
}

void BackendConnection::sendMessage(const QJsonObject& message)
{
	if (!m_connected)
	{
		m_pendingMessages.append(message);
	}
	else
	{
		m_webSocket.sendMessage(message);
	}
}

void BackendConnection::onLogInSuccess(const QJsonObject& /*message*/)
{
	emit loggedIn();
}

void BackendConnection::onLogInFailure(const QJsonObject& message)
{
	if (!message.contains("error") || message["error"].type() != QJsonValue::String)
	{
		LOG << "Message" << ARG2(message["type"], "type") << " must have \"error\" string property";
		return;
	}

	const QString error = message["error"].toString();
	LOG << "Message" << ARG2(message["type"], "type") << ARG(error);
	emit logInFailed(error);
}

/*void BackendConnection::onLogOutSuccess(const QJsonObject& message)
{

}

void BackendConnection::onLogOutFailure(const QJsonObject& message)
{

}*/

void BackendConnection::onDialogsLoadSuccess(const QJsonObject& message)
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

	emit dialogsLoaded(result);
}

void BackendConnection::onDialogsLoadFailure(const QJsonObject& message)
{
	if (!message.contains("error") || message["error"].type() != QJsonValue::String)
	{
		LOG << "Message" << ARG2(message["type"], "type") << " must have \"error\" string property";
		return;
	}

	const QString error = message["error"].toString();
	LOG << "Message" << ARG2(message["type"], "type") << ARG(error);
	emit dialogsLoadFailed(error);
}

void BackendConnection::onDialogsUpdateSuccess(const QJsonObject& /*message*/)
{
	emit dialogsUpdated();
}

void BackendConnection::onDialogsUpdateFailure(const QJsonObject& message)
{
	if (!message.contains("error") || message["error"].type() != QJsonValue::String)
	{
		LOG << "Message" << ARG2(message["type"], "type") << " must have \"error\" string property";
		return;
	}

	const QString error = message["error"].toString();
	LOG << "Message" << ARG2(message["type"], "type") << ARG(error);
	emit dialogsUpdateFailed(error);
}

void BackendConnection::onUsersLoadSuccess(const QJsonObject& message)
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

	emit usersLoaded(result);
}

void BackendConnection::onUsersLoadFailure(const QJsonObject& message)
{
	if (!message.contains("error") || message["error"].type() != QJsonValue::String)
	{
		LOG << "Message" << ARG2(message["type"], "type") << " must have \error\" string property";
		return;
	}

	const QString error = message["error"].toString();
	LOG << "Message" << ARG2(message["type"], "type") << ARG(error);
	emit usersLoadFailed(error);
}

void BackendConnection::onUsersUpdateSuccess(const QJsonObject& /*message*/)
{
	emit usersUpdated();
}

void BackendConnection::onUsersUpdateFailure(const QJsonObject& message)
{
	if (!message.contains("error") || message["error"].type() != QJsonValue::String)
	{
		LOG << "Message" << ARG2(message["type"], "type") << " must have \error\" string property";
		return;
	}

	const QString error = message["error"].toString();
	LOG << "Message" << ARG2(message["type"], "type") << ARG(error);
	emit usersUpdateFailed(error);
}

}
