#pragma once

#include "optional.h"
#include <QString>

namespace Core
{

struct User
{
	enum class Role
	{
		Admin = 0,
		ClientUser,
		ClientGroupSupervisor,
		ClientSupervisor
	};

	User() = default;

	User(const QString& name, const Role& role, bool banned = false, const QString& clientId = {}, const QList<QString>& groups = {})
		: name(name)
		, role(role)
		, clientId(clientId)
		, groups(groups)
		, banned(banned)
	{
	}

	QString name;
	Optional<QString> password;
	Role role { Role::ClientUser };
	QString clientId;
	QList<QString> groups;
	bool banned { false };
};

inline bool operator<(const User& left, const User& right)
{
	return left.name < right.name;
}

inline bool operator==(const User& left, const User& right)
{
	return left.name == right.name && left.role == right.role && left.clientId == right.clientId && left.groups == right.groups && left.banned == right.banned;
}

inline bool operator!=(const User& left, const User& right)
{
	return !(left == right);
}

}
