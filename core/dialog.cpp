#include "dialog.h"
#include "logger.h"

namespace Core
{

Dialog::Dialog()
{
}

Dialog::Dialog(const QString& name, Difficulty difficulty, const QList<PhaseNode>& phases)
	: name(name)
	, difficulty(difficulty)
	, phases(phases)
{
}

Dialog::~Dialog()
{
}

QString Dialog::printableName() const
{
	return name + " (" + difficultyToString(difficulty) + ")";
}

QString Dialog::difficultyToString(Difficulty difficulty)
{
	if (difficulty == Difficulty::Easy)
	{
		return "простой";
	}

	if (difficulty == Difficulty::Hard)
	{
		return "сложный";
	}

	Q_ASSERT(!"Unexpected difficulty");
	return QString();
}

Dialog::Difficulty Dialog::difficultyFromString(const QString& string)
{
	if (string.compare(difficultyToString(Difficulty::Easy), Qt::CaseInsensitive) == 0)
	{
		return Difficulty::Easy;
	}

	if (string.compare(difficultyToString(Difficulty::Hard), Qt::CaseInsensitive) == 0)
	{
		return Difficulty::Hard;
	}

	Q_ASSERT(!"Unexpected difficulty");
}

QStringList Dialog::availableDifficulties()
{
	return { difficultyToString(Difficulty::Easy), difficultyToString(Difficulty::Hard) };
}

bool operator<(const Dialog& left, const Dialog& right)
{
	return left.printableName() < right.printableName();
}

}
