#pragma once

#include <QString>

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

	QString name;
	bool admin;
};

inline bool operator<(const User& left, const User& right)
{
	return left.name < right.name;
}

inline bool operator==(const User& left, const User& right)
{
	return left.name == right.name && left.admin == right.admin;
}

inline bool operator!=(const User& left, const User& right)
{
	return !(left == right);
}

}
