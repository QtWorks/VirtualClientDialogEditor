#include "inmemorybackendconnection.h"
#include "dialogjsonreader.h"
#include "logger.h"

#include <QTimer>
#include <QFile>

namespace Core
{

static const int c_delayMsecs = 2000;

static const QList<User> c_users = {
	User("test", false),
	User("admin", true),
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
	qsrand(QDateTime::currentMSecsSinceEpoch());
}

void InMemoryBackendConnection::logIn(const QString& login, const QString& password)
{
	if (login == "admin" && password == "admin")
	{
		DELAYED_EMIT(loggedIn);
	}
	else
	{
		DELAYED_EMIT_ARGS(logInFailed, "Неправильное имя пользователя или пароль");
	}
}

void InMemoryBackendConnection::logOut()
{
}

void InMemoryBackendConnection::loadDialogs()
{
	if (qrand() % 100 < 50)
	{
		DELAYED_EMIT_ARGS(dialogsLoaded, m_dialogs);
	}
	else
	{
		DELAYED_EMIT_ARGS(dialogsLoadFailed, "Какая-то ошибка загрузки диалогов");
	}
}

void InMemoryBackendConnection::updateDialogs(const Update<Dialog>& update)
{
	if (qrand() % 100 < 50)
	{
		LOG << "Update dialogs: " << update.updated.size() << " updated, " << update.deleted.size() << " deleted, " << update.added.size() << " added";
		DELAYED_EMIT(dialogsUpdated);
	}
	else
	{
		DELAYED_EMIT_ARGS(dialogsUpdateFailed, "Какая-то ошибка обновления данных");
	}
}

void InMemoryBackendConnection::loadUsers()
{
	if (qrand() % 100 < 50)
	{
		DELAYED_EMIT_ARGS(usersLoaded, m_users);
	}
	else
	{
		DELAYED_EMIT_ARGS(usersLoadFailed, "Какая-то ошибка загрузки пользователей");
	}
}

void InMemoryBackendConnection::updateUsers(const Update<User>& update)
{
	if (qrand() % 100 < 50)
	{
		LOG << "Update users: " << update.updated.size() << " updated, " << update.deleted.size() << " deleted, " << update.added.size() << " added";
		DELAYED_EMIT(usersUpdated);
	}
	else
	{
		DELAYED_EMIT_ARGS(usersUpdateFailed, "Какая-то ошибка обновления пользователей");
	}
}

}
