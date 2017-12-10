#pragma once

#include "phase.h"
#include "replica.h"
#include <QString>
#include <QList>

class Dialog
{
public:
	enum class Difficulty
	{
		Easy,
		Hard
	};

	Dialog();
	Dialog(const QString& name, Difficulty difficulty, const QList<Phase>& phases);

	QString printableName() const;

	static Replica errorReplica();
	static Replica goodbyeReplica();

	static QString difficultyToString(Difficulty difficulty);
	static Difficulty difficultyFromString(const QString& string);
	static QStringList availableDifficulties();

	bool operator==(const Dialog& other) const;

	QString name;
	Difficulty difficulty;
	QList<Phase> phases;
};
