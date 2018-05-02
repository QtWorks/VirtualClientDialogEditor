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
	virtual void logIn(const QString& login, const QString& password) override;
	virtual void logOut() override;

	virtual void loadDialogs() override;
	virtual void updateDialogs(const Update<Dialog>& update) override;

	virtual void loadUsers() override;
	virtual void updateUsers(const Update<User>& update) override;

private:
	void onWebSocketConnected();
	void onWebSocketDisconnected();
	void onWebSocketMessage(const QJsonObject& message);

	void sendMessage(const QJsonObject& message);

	void onLogInSuccess(const QJsonObject& message);
	void onLogInFailure(const QJsonObject& message);
	void onLogOutSuccess(const QJsonObject& message);
	void onLogOutFailure(const QJsonObject& message);
	void onDialogsLoadSuccess(const QJsonObject& message);
	void onDialogsLoadFailure(const QJsonObject& message);
	void onDialogsUpdateSuccess(const QJsonObject& message);
	void onDialogsUpdateFailure(const QJsonObject& message);
	void onUsersLoadSuccess(const QJsonObject& message);
	void onUsersLoadFailure(const QJsonObject& message);
	void onUsersUpdateSuccess(const QJsonObject& message);
	void onUsersUpdateFailure(const QJsonObject& message);

private:
	WebSocket m_webSocket;
	bool m_connected;
	QVector<QJsonObject> m_pendingMessages;

	QMap<QString, std::function<void(const QJsonObject&)>> m_messageProcessors;
};

}
