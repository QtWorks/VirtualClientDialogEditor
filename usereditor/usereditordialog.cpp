#include "usereditordialog.h"
#include "ui_usereditordialog.h"
#include <QPushButton>

UserEditorDialog::UserEditorDialog(const Core::User& user, QWidget* parent)
	: QDialog(parent)
	, m_ui(new Ui::UserEditorDialog)
{
	m_ui->setupUi(this);

	m_ui->usernameEdit->setText(user.name);
	m_ui->adminCheckBox->setChecked(user.admin);

	m_ui->buttonBox->button(QDialogButtonBox::Save)->setText("Сохранить");
	m_ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отменить");
	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &UserEditorDialog::saveChanges);
	connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &UserEditorDialog::discardChanges);
}

UserEditorDialog::~UserEditorDialog()
{
	delete m_ui;
}

void UserEditorDialog::saveChanges()
{
	const QString username = m_ui->usernameEdit->text();
	const bool admin = m_ui->adminCheckBox->isChecked();

	emit userChanged({ username, admin });
	accept();
}

void UserEditorDialog::discardChanges()
{
	reject();
}
