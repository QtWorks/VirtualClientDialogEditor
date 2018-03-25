#include "dialogmodel.h"

DialogModel::DialogModel(const Dialog& dialog)
	: m_dialog(dialog)
{
}

QString DialogModel::name() const
{
	return m_dialog.name;
}

void DialogModel::setName(const QString& name)
{
	// TODO: add validation?
	m_dialog.name = name;
}

Dialog::Difficulty DialogModel::difficulty() const
{
	return m_dialog.difficulty;
}

void DialogModel::setDifficulty(Dialog::Difficulty difficulty)
{
	// TODO: add validation?
	m_dialog.difficulty = difficulty;
}

QList<Phase> DialogModel::phases() const
{
	return m_dialog.phases;
}
