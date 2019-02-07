#pragma once

#include "ui_groupsdialog.h"
#include <core/client.h>

class GroupsDialog
	: public QDialog
{
	Q_OBJECT

public:
	GroupsDialog(const QList<Core::Group>& items, const QList<QString>& checkedItems, QWidget* parent = 0);

signals:
	void accepted(QList<QString> groups);

private slots:
	void saveChanges();
	void discardChanges();

private:
	Ui::GroupsDialog m_ui;
};
