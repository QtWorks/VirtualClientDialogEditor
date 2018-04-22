#ifndef INMEMORYBACKENDCONNECTION_H
#define INMEMORYBACKENDCONNECTION_H

#include "ibackendconnection.h"

namespace Core
{

class InMemoryBackendConnection
	: public IBackendConnection
{
public:
	InMemoryBackendConnection();

private:
	virtual void logIn(const QString& login, const QString& password) override;
	virtual void logOut() override;

	virtual void loadDialogs() override;
	virtual void updateDialogs(const Update<Dialog>& update) override;

	virtual void loadUsers() override;
	virtual void updateUsers(const Update<User>& update) override;

private:
	QList<Dialog> m_dialogs;
	QList<User> m_users;
};

}

#endif // INMEMORYBACKENDCONNECTION_H
