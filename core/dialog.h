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
	~Dialog();

	QString printableName() const;

	// TODO: error replica?
	// TODO: intro replica?
	// TODO: outro replica?

	static QString difficultyToString(Difficulty difficulty);

	static Difficulty difficultyFromString(const QString& string);
	static QStringList availableDifficulties();

	QString name;
	Difficulty difficulty;
	QList<PhaseNode> phases;
};

}

#endif // DIALOG_H
