#pragma once

#include <QString>

namespace Core
{

struct Client
{
	typedef QByteArray Id;

	Client() = default;

	Client(const QString& name, const QString& databaseName, const Id& id)
		: name(name)
		, databaseName(databaseName)
		, id(id)
	{
	}

	QString name;
	QString databaseName;
	QString id;
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
