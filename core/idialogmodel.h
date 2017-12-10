#ifndef IDIALOGMODEL_H
#define IDIALOGMODEL_H

//#include "itransitioncondition.h"
#include "dialog.h"
#include <QList>

class IDialogModel
{
public:
	/*struct Node
	{
		QList<Node*> parentNodes; // ???
		QString data;
		QList<QPair<Node*, ITransitionCondition*>> childNodes;
	};*/

	virtual ~IDialogModel() {}

	//virtual const Node& root() const = 0;
	//virtual const Node& errorNode() const = 0;

	//virtual void addChild(const Node& node, const Node& childNode) = 0;
	//virtual void removeChild(const Node& node, const Node& childNode) = 0;

	/*
	static Replica errorReplica();
	static Replica goodbyeReplica();

	static QString difficultyToString(Difficulty difficulty);
	static Difficulty difficultyFromString(const QString& string);
	static QStringList availableDifficulties();

	bool operator==(const Dialog& other) const;

	QString name;
	Difficulty difficulty;
	QList<Phase> phases;*/

	virtual QString name() const = 0;
	virtual Dialog::Difficulty difficulty() const = 0;
	virtual QList<Phase> phases() const = 0;
};

#endif // IDIALOGMODEL_H
