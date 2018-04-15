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
	virtual void logIn(const QString& url, const QString& login, const QString& password) override;
	virtual void logOut() override;

	virtual void readDialogs() override;
	virtual void addDialog(const Dialog& dialog) override;
	virtual void updateDialog(const QString& name, Dialog::Difficulty difficulty, const Dialog& dialog) override;
	virtual void deleteDialog(const QString& name, Dialog::Difficulty difficulty) override;

	virtual void readUsers() override;
	virtual void addUser(const User& user) override;
	virtual void updateUser(const User::UsernameType& username, const User& user) override;
	virtual void deleteUser(const User::UsernameType& username) override;

private:
	QList<Dialog> m_dialogs;
	QList<User> m_users;
};

}

#endif // INMEMORYBACKENDCONNECTION_H
