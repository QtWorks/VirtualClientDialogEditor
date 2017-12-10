#pragma once

#include <QList>
#include <QString>

class Replica
{
public:
	Replica();
	Replica(const QString& clientReplica, const QStringList& expectedWords, const QString& hint);

	bool operator==(const Replica& other) const;

	QString clientReplica;
	QStringList expectedWords;
	QString hint;
};
