#include "groupsdialog.h"

#include <QPushButton>
#include <QMessageBox>

GroupsDialog::GroupsDialog(const QList<Core::Group>& items, const QList<QString>& checkedItems, QWidget* parent)
	: QDialog(parent)
{
	m_ui.setupUi(this);
	setModal(true);

	m_ui.groupsListWidget->setItems(items);
	m_ui.groupsListWidget->setCheckedItems(checkedItems);

	m_ui.buttonBox->button(QDialogButtonBox::Save)->setText("Сохранить");
	connect(m_ui.buttonBox, &QDialogButtonBox::accepted, this, &GroupsDialog::saveChanges);

	m_ui.buttonBox->button(QDialogButtonBox::Cancel)->setText("Отменить");
	connect(m_ui.buttonBox, &QDialogButtonBox::rejected, this, &GroupsDialog::discardChanges);
}

void GroupsDialog::saveChanges()
{
	QList<QString> checkedGroups = m_ui.groupsListWidget->checkedItems();

	if (checkedGroups.isEmpty())
	{
		QMessageBox::warning(this, "Ошибка валидации", "Необходимо выбрать хотя бы одну группу.");
		return;
	}

	emit accepted(checkedGroups);

	accept();
}

void GroupsDialog::discardChanges()
{
	reject();
}
