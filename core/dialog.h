#ifndef DIALOG_H
#define DIALOG_H

#include "phasenode.h"
#include "clientreplicanode.h"
#include "expectedwordsnode.h"
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
	Dialog(const QString& name, Difficulty difficulty, const QList<PhaseNode>& phases);
	Dialog(const Dialog& other);
	~Dialog();

	QString printableName() const;

	static QString difficultyToString(Difficulty difficulty);

	static Difficulty difficultyFromString(const QString& string);
	static QStringList availableDifficulties();

	QString name;
	Difficulty difficulty;
	QList<PhaseNode> phases;
};

bool operator<(const Dialog& left, const Dialog& right);

bool operator==(const Dialog& left, const Dialog& right);
bool operator!=(const Dialog& left, const Dialog& right);

}

#endif // DIALOG_H
