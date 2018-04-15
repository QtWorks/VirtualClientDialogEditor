#include "inmemorybackendconnection.h"
#include "dialogjsonreader.h"
#include "logger.h"

#include <QTimer>
#include <QFile>

namespace Core
{

static const int c_delayMsecs = 20;

static const QList<User> c_users = {
	User("test", User::Permissions(true, false)),
	User("admin", User::Permissions(true, true)),
};

#define DELAYED_EMIT(signal) QTimer::singleShot(c_delayMsecs, [this]() { emit (signal)(); });
#define DELAYED_EMIT_ARGS(signal, args) QTimer::singleShot(c_delayMsecs, [&]() { emit (signal)(args); });

Dialog readTestDialog(const QString& name, bool& ok)
{
	QFile file(name);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		LOG << ARG(name) << ARG2(file.errorString(), "error");
		ok = false;
		return Dialog();
	}

	const QByteArray fileContent = file.readAll();
	return DialogJsonReader().read(fileContent, ok);
}

QList<Dialog> readTestDialogs()
{
	QList<Dialog> results;

	const QVector<QString> files = { "D:\\test_dialog_2.json", "D:\\test_dialog_3.json", "D:\\test_dialog_4.json" };

	bool ok = false;
	for (const QString& file : files)
	{
		Dialog dialog = readTestDialog(file, ok);
		if (ok)
		{
			results << dialog;
		}
	}

	return results;
}

InMemoryBackendConnection::InMemoryBackendConnection()
	: m_dialogs(readTestDialogs())
	, m_users(c_users)
{
}

void InMemoryBackendConnection::logIn(const QString& /*url*/, const QString& login, const QString& password)
{
	if (login == "admin" && password == "admin")
	{
		DELAYED_EMIT(loggedIn);
	}
	else
	{
		DELAYED_EMIT_ARGS(onError, "Неправильное имя пользователя или пароль");
	}
}

void InMemoryBackendConnection::logOut()
{
}

void InMemoryBackendConnection::readDialogs()
{
	DELAYED_EMIT_ARGS(onDialogsReaded, m_dialogs);
}

void InMemoryBackendConnection::addDialog(const Dialog& /*dialog*/)
{
}

void InMemoryBackendConnection::updateDialog(const QString& /*name*/, Dialog::Difficulty /*difficulty*/, const Dialog& /*dialog*/)
{
}

void InMemoryBackendConnection::deleteDialog(const QString& /*name*/, Dialog::Difficulty /*difficulty*/)
{
}

void InMemoryBackendConnection::readUsers()
{
	DELAYED_EMIT_ARGS(onUsersReaded, m_users);
}

void InMemoryBackendConnection::addUser(const User& /*user*/)
{
}

void InMemoryBackendConnection::updateUser(const User::UsernameType& /*username*/, const User& /*user*/)
{
}

void InMemoryBackendConnection::deleteUser(const User::UsernameType& /*username*/)
{
}

}
