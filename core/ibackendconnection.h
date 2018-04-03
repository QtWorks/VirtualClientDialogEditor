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

class IBackendConnection
	: public QObject
{
	Q_OBJECT

public:
	virtual ~IBackendConnection() { }

	virtual void open(const QString& url, const QString& login, const QString& password) = 0;
	virtual void close() = 0;

	virtual void readDialogs() = 0;
	virtual void addDialog(const Dialog& dialog) = 0;
	virtual void updateDialog(const QString& name, Dialog::Difficulty difficulty, const Dialog& dialog) = 0;
	virtual void deleteDialog(const QString& name, Dialog::Difficulty difficulty) = 0;

	virtual void readUsers() = 0;
	virtual void addUser(const User& user) = 0;
	virtual void updateUser(const QString& username, const User& user) = 0;
	virtual void deleteUser(const QString& username) = 0;

	virtual void triggerError(const QString& message) = 0;

signals:
	void onAuth();
	void onConnectionClosed(const QString& reason);

	void onDialogsReaded(const QList<Dialog>& dialogs);
	void onDialogAdded();
	void onDialogUpdated();
	void onDialogRemoved();

	void onUsersReaded(const QList<User>& users);
	void onUserAdded();
	void onUserUpdated();
	void onUserDeleted();

	void onError(const QString& message);
};

}

Q_DECLARE_INTERFACE(Core::IBackendConnection, "IBackendConnection")

typedef std::unique_ptr<Core::IBackendConnection> IBackendConnectionUniquePtr;
typedef std::shared_ptr<Core::IBackendConnection> IBackendConnectionSharedPtr;

#endif // IBACKENDCONNECTION_H
