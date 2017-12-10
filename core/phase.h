#pragma once

#include "replica.h"
#include <QString>
#include <QList>

class Phase
{
public:
	Phase();
	Phase(const QString& name, const QList<Replica>& replicas);

	bool operator==(const Phase& other) const;

	QString name;
	QList<Replica> replicas;
};
