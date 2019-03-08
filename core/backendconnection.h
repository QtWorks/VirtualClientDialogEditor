#pragma once

#include "ibackendconnection.h"
#include "websocket.h"
#include "optional.h"
#include <functional>

namespace Core
{

class BackendConnection
	: public IBackendConnection
{
public:
	BackendConnection(const QUrl& url);
	virtual ~BackendConnection();

private:
	virtual QueryId logIn(const QString& login, const QString& password) override;
	virtual QueryId logOut() override;

	virtual QueryId loadClients() override;
	virtual QueryId updateClients(const Update<Client>& update) override;

	virtual QueryId loadUsers() override;
	virtual QueryId updateUsers(const Update<User>& update) override;

	virtual QueryId loadDialogs() override;
	virtual QueryId updateDialogs(const QString& cliendId, const Update<Dialog>& update) override;

	virtual QueryId cleanupClientStatistics(const QString& clientId) override;
	virtual QueryId cleanupGroupStatistics(const QString& clientId, const QString& groupName) override;
	virtual QueryId cleanupUserStatistics(const QString& clientId, const QString& username) override;

private:
	void onWebSocketDisconnected();
	void onWebSocketError(const QString& errorMessage);
	void onWebSocketMessage(const QJsonObject& message);

	QueryId sendMessage(const QJsonObject& message);

	void onLogInSuccess(IBackendConnection::QueryId queryId);
	void onLogInFailure(IBackendConnection::QueryId queryId, const QJsonObject& message);

	void onLogOutSuccess(IBackendConnection::QueryId queryId);
	void onLogOutFailure(IBackendConnection::QueryId queryId, const QJsonObject& message);

	void onClientsLoadSuccess(IBackendConnection::QueryId queryId, const QJsonObject& message);
	void onClientsLoadFailure(IBackendConnection::QueryId queryId, const QJsonObject& message);

	void onClientsUpdateSuccess(IBackendConnection::QueryId queryId);
	void onClientsUpdateFailure(IBackendConnection::QueryId queryId, const QJsonObject& message);

	void onUsersLoadSuccess(IBackendConnection::QueryId queryId, const QJsonObject& message);
	void onUsersLoadFailure(IBackendConnection::QueryId queryId, const QJsonObject& message);

	void onUsersUpdateSuccess(IBackendConnection::QueryId queryId);
	void onUsersUpdateFailure(IBackendConnection::QueryId queryId, const QJsonObject& message);

	void onDialogsLoadSuccess(IBackendConnection::QueryId queryId, const QJsonObject& message);
	void onDialogsLoadFailure(IBackendConnection::QueryId queryId, const QJsonObject& message);

	void onDialogsUpdateSuccess(IBackendConnection::QueryId queryId);
	void onDialogsUpdateFailure(IBackendConnection::QueryId queryId, const QJsonObject& message);

	void onStatisticsCleanupSuccess(IBackendConnection::QueryId queryId);
	void onStatisticsCleanupFailure(IBackendConnection::QueryId queryId, const QJsonObject& message);


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
