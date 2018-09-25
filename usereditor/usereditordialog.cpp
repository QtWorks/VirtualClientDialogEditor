#include "usereditordialog.h"
#include "ui_usereditordialog.h"
#include <QPushButton>
#include <QMessageBox>

UserEditorDialog::UserEditorDialog(const Core::User& user, const UniquenessValidator& validator, const QList<Core::Client>& clients, QWidget* parent)
	: QDialog(parent)
	, m_ui(new Ui::UserEditorDialog)
	, m_uniquenessValidator(validator)
	, m_clients(clients)
{
	m_ui->setupUi(this);
	setModal(true);

	m_ui->usernameEdit->setText(user.name);

	for (const Core::Client& client : clients)
	{
		m_ui->clientsComboBox->addItem(client.name, client.id);
	}

	m_ui->adminCheckBox->setChecked(user.admin);

	if (!user.admin)
	{
		const auto it = std::find_if(clients.begin(), clients.end(),
			[&user](const Core::Client& client) { return user.clientId == client.id; });

		const int index = it == clients.end() ? -1 : std::distance(clients.begin(), it);
		m_ui->clientsComboBox->setCurrentIndex(index);
	}

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
	const QString username = m_ui->usernameEdit->text().trimmed();
	if (username.isEmpty())
	{
		QMessageBox::warning(this, "Ошибка валидации", "Имя пользователя должно быть не пустым");
		return;
	}

	if (!m_uniquenessValidator(username))
	{
		QMessageBox::warning(this, "Ошибка валидации", "Имя пользователя должно быть уникальным");
		return;
	}

	const bool admin = m_ui->adminCheckBox->isChecked();
	if (admin)
	{
		emit userChanged({ username, admin });
	}
	else
	{
		const int clientIndex = m_ui->clientsComboBox->currentIndex();
		const QString clientId = m_clients[clientIndex].id;
		emit userChanged({ username, clientId});
	}

	accept();
}

void UserEditorDialog::discardChanges()
{
	reject();
}
