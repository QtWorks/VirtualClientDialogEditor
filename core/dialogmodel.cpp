#include "dialogmodel.h"

DialogModel::DialogModel(const Dialog& dialog)
	: m_dialog(dialog)
{
}

QString DialogModel::name() const
{
	return m_dialog.name;
}

Dialog::Difficulty DialogModel::difficulty() const
{
	return m_dialog.difficulty;
}

QList<Phase> DialogModel::phases() const
{
	return m_dialog.phases;
}
