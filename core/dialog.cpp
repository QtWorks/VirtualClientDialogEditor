#include "dialog.h"
#include "logger.h"

namespace Core
{

Dialog::Dialog()
{
}

Dialog::Dialog(const QString& name, Difficulty difficulty, const QList<PhaseNode>& phases, const ErrorReplica& errorReplica)
	: name(name)
	, difficulty(difficulty)
	, phases(phases)
	, errorReplica(errorReplica)
{
}

Dialog::Dialog(const Dialog& other)
	: name(other.name)
	, difficulty(other.difficulty)
	, errorReplica(other.errorReplica)
	, phaseRepeatReplica(other.phaseRepeatReplica)
{
	for (const PhaseNode& phase : other.phases)
	{
		phases.append(PhaseNode(phase));
	}
}

Dialog::~Dialog()
{
}

QString Dialog::printableName() const
{
	return printableName(name, difficulty);
}

QString Dialog::printableName(const QString& name, Difficulty difficulty)
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

bool operator==(const Dialog& left, const Dialog& right)
{
	return left.name == right.name && left.difficulty == right.difficulty &&
		left.phases.size() == right.phases.size() &&
		std::equal(left.phases.begin(), left.phases.end(), right.phases.begin()) &&
		left.errorReplica == right.errorReplica &&
		left.phaseRepeatReplica == right.phaseRepeatReplica;
}

bool operator!=(const Dialog& left, const Dialog& right)
{
	return !(left == right);
}

}
