#include "usereditordialog.h"
#include "ui_usereditordialog.h"

UserEditorDialog::UserEditorDialog(const Core::User& user, QWidget* parent)
	: QDialog(parent)
	, m_ui(new Ui::UserEditorDialog)
{
	m_ui->setupUi(this);

	m_ui->usernameEdit->setText(user.name());
	m_ui->writePermissionCheckBox->setChecked(user.permissions().write);

	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, [this]()
	{
		// TODO: emptiness validation, trims
		const QString username = m_ui->usernameEdit->text();
		const Core::User::Permissions permissions = { true, m_ui->writePermissionCheckBox->isChecked() };

		emit userChanged({ username, permissions });
		accept();
	});

	connect(m_ui->buttonBox, &QDialogButtonBox::rejected, [this]()
	{
		reject();
	});
}

UserEditorDialog::~UserEditorDialog()
{
	delete m_ui;
}
