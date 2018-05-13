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

	typedef int QueryId;

	virtual QueryId logIn(const QString& login, const QString& password) = 0;
	virtual QueryId logOut() = 0;

	virtual QueryId loadDialogs() = 0;
	virtual QueryId updateDialogs(const Update<Dialog>& update) = 0;

	virtual QueryId loadUsers() = 0;
	virtual QueryId updateUsers(const Update<User>& update) = 0;

signals:
	void loggedIn(QueryId queryId);
	void logInFailed(QueryId queryId, const QString& error);

	void dialogsLoaded(QueryId queryId, const QList<Dialog>& dialogs);
	void dialogsLoadFailed(QueryId queryId, const QString& error);

	void dialogsUpdated(QueryId queryId);
	void dialogsUpdateFailed(QueryId queryId, const QString& error);

	void usersLoaded(QueryId queryId, const QList<User>& users);
	void usersLoadFailed(QueryId queryId, const QString& error);

	void usersUpdated(QueryId queryId);
	void usersUpdateFailed(QueryId queryId, const QString& error);
};

}

Q_DECLARE_INTERFACE(Core::IBackendConnection, "IBackendConnection")

typedef std::shared_ptr<Core::IBackendConnection> IBackendConnectionSharedPtr;

#endif // IBACKENDCONNECTION_H
