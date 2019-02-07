#pragma once

#include "ui_groupeditordialog.h"
#include "core/client.h"

#include <QDialog>
#include <functional>

class GroupEditorDialog
	: public QDialog
{
	Q_OBJECT

public:
	typedef std::function<bool(const QString&)> GroupNameValidator;
	GroupEditorDialog(const Core::Group& group, const GroupNameValidator& groupNameValidator, QWidget* parent = 0);
	~GroupEditorDialog();

signals:
	void groupChanged(Core::Group group);
	void canceled();

private slots:
	void saveChanges();
	void discardChanges();

private:
	Ui::GroupEditorDialog* m_ui;

	Core::Group m_group;
	GroupNameValidator m_groupNameValidator;
};
