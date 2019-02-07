#pragma once

#include <QString>
#include <QList>

namespace Core
{

struct Group
{
	Group() = default;

	Group(const QString& name, const QString& id)
		: name(name)
		, id(id)
	{
	}

	QString name;
	QString id;
};

inline bool operator==(const Group& left, const Group& right)
{
	return left.name == right.name && left.id == right.id;
}

struct Client
{
	Client() = default;

	Client(const QString& name, const QString& databaseName, const QByteArray& id, const QList<Group>& groups)
		: name(name)
		, databaseName(databaseName)
		, id(id)
		, groups(groups)
	{
	}

	QString name;
	QString databaseName;
	QByteArray id;
	QList<Group> groups;
};

inline bool operator<(const Client& left, const Client& right)
{
	return left.name < right.name && left.databaseName < right.databaseName && left.id < right.id;
}

inline bool operator==(const Client& left, const Client& right)
{
	return left.name == right.name && left.databaseName == right.databaseName && left.id == right.id;
}

inline bool operator!=(const Client& left, const Client& right)
{
	return !(left == right);
}

}
