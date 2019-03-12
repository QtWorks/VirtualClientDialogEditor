#include "clienteditordialog.h"
#include "ui_clienteditordialog.h"
#include <QPushButton>
#include <QMessageBox>

ClientEditorDialog::ClientEditorDialog(const Core::Client& client, const UniquenessValidator& nameValidator,
	const UniquenessValidator& databaseNameValidator, QWidget* parent)
	: QDialog(parent)
	, m_ui(new Ui::ClientEditorDialog)
	, m_client(client)
	, m_nameValidator(nameValidator)
	, m_databaseNameValidator(databaseNameValidator)
{
	m_ui->setupUi(this);
	setModal(true);

	m_ui->nameLineEdit->setText(client.name);
	m_ui->databaseNameLineEdit->setText(client.databaseName);

	m_ui->buttonBox->button(QDialogButtonBox::Save)->setText("Сохранить");
	m_ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отменить");
	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &ClientEditorDialog::saveChanges);
	connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &ClientEditorDialog::discardChanges);
}

ClientEditorDialog::~ClientEditorDialog()
{
	delete m_ui;
}

void ClientEditorDialog::saveChanges()
{
	const QString name = m_ui->nameLineEdit->text().trimmed();
	if (name.isEmpty())
	{
		QMessageBox::warning(this, "Ошибка валидации", "Имя клиента должно быть не пустым");
		return;
	}

	if (!m_nameValidator(name))
	{
		QMessageBox::warning(this, "Ошибка валидации", "Имя клиента должно быть уникальным");
		return;
	}

	const QString databaseName = m_ui->databaseNameLineEdit->text().trimmed();
	if (databaseName.isEmpty())
	{
		QMessageBox::warning(this, "Ошибка валидации", "Имя базы клиента должно быть не пустым");
		return;
	}

	if (!m_databaseNameValidator(name))
	{
		QMessageBox::warning(this, "Ошибка валидации", "Имя базы клиента должно быть уникальным");
		return;
	}

	emit clientChanged({ name, databaseName, "", m_client.groups, m_client.banned });
	accept();
}

void ClientEditorDialog::discardChanges()
{
	reject();
}
