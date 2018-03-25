#include "dialog.h"

Dialog::Dialog()
{
}

Dialog::Dialog(const QString& name, Difficulty difficulty, const QList<Phase>& phases)
	: name(name)
	, difficulty(difficulty)
	, phases(phases)
{
}

QString Dialog::printableName() const
{
	return name + " (" + difficultyToString(difficulty) + ")";
}

Replica Dialog::errorReplica()
{
	return Replica("Вы знаете, у меня еще много работы. Если Вы считаете, что визит окончен, будем прощаться?", {}, "");
}

Replica Dialog::goodbyeReplica()
{
	return Replica("Визит окончен, до свидания", { "До свидания", "Пока", "До скорой встречи" }, "");
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

bool Dialog::operator==(const Dialog& other) const
{
	if (&other == this)
	{
		return true;
	}

	if (name.compare(other.name, Qt::CaseInsensitive) != 0 ||
		other.difficulty != difficulty)
	{
		return false;
	}

	return other.phases == phases;
}
