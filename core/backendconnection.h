#pragma once

#include "ibackendconnection.h"
#include "websocket.h"
#include <functional>

namespace Core
{

class BackendConnection
	: public IBackendConnection
{
public:
	BackendConnection(const QUrl& url);

private:
	virtual QueryId logIn(const QString& login, const QString& password) override;
	virtual QueryId logOut() override;

	virtual QueryId loadDialogs() override;
	virtual QueryId updateDialogs(const Update<Dialog>& update) override;

	virtual QueryId loadUsers() override;
	virtual QueryId updateUsers(const Update<User>& update) override;

private:
	void onWebSocketDisconnected();
	void onWebSocketError(const QString& errorMessage);
	void onWebSocketMessage(const QJsonObject& message);

	QueryId sendMessage(const QJsonObject& message);

	void onLogInSuccess(IBackendConnection::QueryId queryId);
	void onLogInFailure(IBackendConnection::QueryId queryId, const QJsonObject& message);

	void onLogOutSuccess(IBackendConnection::QueryId queryId);
	void onLogOutFailure(IBackendConnection::QueryId queryId, const QJsonObject& message);

	void onDialogsLoadSuccess(IBackendConnection::QueryId queryId, const QJsonObject& message);
	void onDialogsLoadFailure(IBackendConnection::QueryId queryId, const QJsonObject& message);

	void onDialogsUpdateSuccess(IBackendConnection::QueryId queryId);
	void onDialogsUpdateFailure(IBackendConnection::QueryId queryId, const QJsonObject& message);

	void onUsersLoadSuccess(IBackendConnection::QueryId queryId, const QJsonObject& message);
	void onUsersLoadFailure(IBackendConnection::QueryId queryId, const QJsonObject& message);

	void onUsersUpdateSuccess(IBackendConnection::QueryId queryId);
	void onUsersUpdateFailure(IBackendConnection::QueryId queryId, const QJsonObject& message);

private:
	WebSocket m_webSocket;

	struct Processor
	{
		typedef std::function<void(IBackendConnection::QueryId queryId, const QJsonObject& message)> ProcessMessage;
		typedef std::function<void(IBackendConnection::QueryId queryId, const QString&)> ProcessWebSocketDisconnect;
		typedef std::function<void(IBackendConnection::QueryId queryId, const QString& errorMessage)> ProcessWebSocketError;

		Processor()
			: processData([](IBackendConnection::QueryId, const QJsonObject&) { Q_ASSERT(!"Not implemented"); })
			, processError([](IBackendConnection::QueryId, const QJsonObject&) { Q_ASSERT(!"Not implemented"); })
			, processWebSocketDisconnect([](IBackendConnection::QueryId, const QString&) { Q_ASSERT(!"Not implemented"); })
			, processWebSocketError([](IBackendConnection::QueryId, const QString&) { Q_ASSERT(!"Not implemented"); })
		{
		}

		Processor(ProcessMessage processData, ProcessMessage processError, ProcessWebSocketDisconnect processWebSocketDisconnect, ProcessWebSocketError processWebSocketError)
			: processData(processData)
			, processError(processError)
			, processWebSocketDisconnect(processWebSocketDisconnect)
			, processWebSocketError(processWebSocketError)
		{
		}

		ProcessMessage processData;
		ProcessMessage processError;
		ProcessWebSocketDisconnect processWebSocketDisconnect;
		ProcessWebSocketError processWebSocketError;
	};

	Processor makeProcessor(const QString& queryType);

	QMap<IBackendConnection::QueryId, Processor> m_activeQueries;
};

}
