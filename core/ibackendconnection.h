#ifndef IBACKENDCONNECTION_H
#define IBACKENDCONNECTION_H

#include "dialog.h"
#include "user.h"

#include <QObject>
#include <QString>
#include <QList>

#include <memory>

namespace Core
{

template <typename T>
struct Update
{
	QMap<T, T> updated;
	QList<T> deleted;
	QList<T> added;
};

class IBackendConnection
	: public QObject
{
	Q_OBJECT

public:
	virtual ~IBackendConnection() { }

	virtual void logIn(const QString& login, const QString& password) = 0;
	virtual void logOut() = 0;

	virtual void loadDialogs() = 0;
	virtual void updateDialogs(const Update<Dialog>& update) = 0;

	virtual void loadUsers() = 0;
	virtual void updateUsers(const Update<User>& update) = 0;

signals:
	void loggedIn();
	void logInFailed(const QString& error);

	void dialogsLoaded(const QList<Dialog>& dialogs);
	void dialogsLoadFailed(const QString& error);

	void dialogsUpdated();
	void dialogsUpdateFailed(const QString& error);

	void usersLoaded(const QList<User>& users);
	void usersLoadFailed(const QString& error);

	void usersUpdated();
	void usersUpdateFailed(const QString& error);

	void onConnectionClosed(const QString& reason);
};

}

Q_DECLARE_INTERFACE(Core::IBackendConnection, "IBackendConnection")

typedef std::shared_ptr<Core::IBackendConnection> IBackendConnectionSharedPtr;

#endif // IBACKENDCONNECTION_H
