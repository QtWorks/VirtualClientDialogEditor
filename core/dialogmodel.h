#ifndef DIALOGMODEL_H
#define DIALOGMODEL_H

#include "idialogmodel.h"
#include "dialog.h"

class DialogModel
	: public IDialogModel
{
public:
	DialogModel(const Dialog& dialog);

private:
	virtual QString name() const override;
	virtual void setName(const QString& name) override;

	virtual Dialog::Difficulty difficulty() const override;
	virtual void setDifficulty(Dialog::Difficulty difficulty) override;

	virtual QList<Phase> phases() const override;

private:
	Dialog m_dialog;
};

#endif // DIALOGMODEL_H
