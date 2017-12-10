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
	virtual Dialog::Difficulty difficulty() const override;
	virtual QList<Phase> phases() const override;

private:
	Dialog m_dialog;
	//Node m_node;
	//Node m_errorNode;
};

#endif // DIALOGMODEL_H
