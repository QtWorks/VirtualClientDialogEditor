#pragma once

#include <QString>
#include <QList>

namespace Core
{

struct Group
{
	Group() = default;

	Group(const QString& name, const QString& id, bool banned)
		: name(name)
		, id(id)
		, banned(banned)
	{
	}

	QString name;
	QString id;
	bool banned { false };
};

inline bool operator==(const Group& left, const Group& right)
{
	return left.name == right.name && left.id == right.id && left.banned == right.banned;
}

struct Client
{
	Client() = default;

	Client(const QString& name, const QString& databaseName, const QByteArray& id, const QList<Group>& groups, bool banned)
		: name(name)
		, databaseName(databaseName)
		, id(id)
		, groups(groups)
		, banned(banned)
	{
	}

	QString name;
	QString databaseName;
	QByteArray id;
	QList<Group> groups;
	bool banned { false };
};

inline bool operator<(const Client& left, const Client& right)
{
	return left.name < right.name && left.databaseName < right.databaseName && left.id < right.id;
}

inline bool operator==(const Client& left, const Client& right)
{
	return left.name == right.name && left.databaseName == right.databaseName && left.id == right.id && left.groups == right.groups && left.banned == right.banned;
}

inline bool operator!=(const Client& left, const Client& right)
{
	return !(left == right);
}

}
