#include "groupeditordialog.h"

#include <QPushButton>
#include <QMessageBox>

GroupEditorDialog::GroupEditorDialog(const Core::Group& group, const GroupNameValidator& groupNameValidator, QWidget* parent)
	: QDialog(parent)
	, m_ui(new Ui::GroupEditorDialog)
	, m_group(group)
	, m_groupNameValidator(groupNameValidator)
{
	m_ui->setupUi(this);
	setModal(true);

	m_ui->groupNameLineEdit->setText(group.name);

	QPushButton* saveButton = m_ui->buttonBox->button(QDialogButtonBox::Save);
	saveButton->setText("Сохранить");
	connect(saveButton, &QPushButton::clicked, this, &GroupEditorDialog::saveChanges);

	QPushButton* discardButton = m_ui->buttonBox->button(QDialogButtonBox::Cancel);
	discardButton->setText("Отменить");
	connect(discardButton, &QPushButton::clicked, this, &GroupEditorDialog::discardChanges);
}

GroupEditorDialog::~GroupEditorDialog()
{
	delete m_ui;
}

void GroupEditorDialog::saveChanges()
{
	const QString name = m_ui->groupNameLineEdit->text().trimmed();
	if (name.isEmpty())
	{
		QMessageBox::warning(this, "Ошибка валидации", "Имя группы должно быть не пустым");
		return;
	}

	if (!m_groupNameValidator(name))
	{
		QMessageBox::warning(this, "Ошибка валидации", "Имя группы должно быть уникальным");
		return;
	}

	emit groupChanged({ name, m_group.id, m_group.banned });
	accept();
}

void GroupEditorDialog::discardChanges()
{
	reject();
}
