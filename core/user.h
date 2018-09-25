#pragma once

#include <QString>
#include <QVariant>

namespace Core
{

struct User
{
	User()
		: admin(false)
	{
	}

	User(const QString& name, bool admin)
		: name(name)
		, admin(admin)
	{
	}

	User(const QString& name, const QString& clientId)
		: name(name)
		, admin(false)
		, clientId(clientId)
	{
	}

	QString name;
	QVariant password;
	bool admin;
	QString clientId;
};

inline bool operator<(const User& left, const User& right)
{
	return left.name < right.name;
}

inline bool operator==(const User& left, const User& right)
{
	return left.name == right.name && left.admin == right.admin && left.clientId == right.clientId;
}

inline bool operator!=(const User& left, const User& right)
{
	return !(left == right);
}

}
