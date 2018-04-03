#ifndef IDIALOGMODEL_H
#define IDIALOGMODEL_H

#include "dialog.h"
#include <QList>

namespace Core
{

class IDialogModel
{
public:
	virtual ~IDialogModel() {}

	virtual QString name() const = 0;
	virtual void setName(const QString& name) = 0;

	virtual Dialog::Difficulty difficulty() const = 0;
	virtual void setDifficulty(Dialog::Difficulty difficulty) = 0;

	virtual QList<PhaseNode> phases() const = 0;
};

}

#endif // IDIALOGMODEL_H
