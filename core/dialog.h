#pragma once

#include "phasenode.h"
#include "clientreplicanode.h"
#include "expectedwordsnode.h"
#include "errorreplica.h"
#include <QString>

namespace Core
{

class Dialog
{
public:
	enum class Difficulty
	{
		Easy,
		Hard
	};

	Dialog();
	Dialog(const QString& name, Difficulty difficulty, const QString& note, const QList<PhaseNode>& phases, const ErrorReplica& errorReplica, double successRatio, QList<QString> groups);
	Dialog(const Dialog& other);
	~Dialog();

	QString printableName() const;
	static QString printableName(const QString& name, Difficulty difficulty);

	static QString difficultyToString(Difficulty difficulty);

	static Difficulty difficultyFromString(const QString& string);
	static QStringList availableDifficulties();

	QString name;
	Difficulty difficulty;
	QString note;
	QList<PhaseNode> phases;
	ErrorReplica errorReplica;
	QVariant phaseRepeatReplica;
	double successRatio;
	QList<QString> groups;
};

bool operator<(const Dialog& left, const Dialog& right);

bool operator==(const Dialog& left, const Dialog& right);
bool operator!=(const Dialog& left, const Dialog& right);

}
