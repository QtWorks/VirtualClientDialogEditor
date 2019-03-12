#include "usereditordialog.h"
#include "ui_usereditordialog.h"
#include <QPushButton>
#include <QMessageBox>

UserEditorDialog::UserEditorDialog(const Core::User& user, const UniquenessValidator& validator, const QList<Core::Client>& clients,
	bool showPasswordInputs, QWidget* parent)
	: QDialog(parent)
	, m_ui(new Ui::UserEditorDialog)
	, m_user(user)
	, m_uniquenessValidator(validator)
	, m_clients(clients)
	, m_showPasswordInputs(showPasswordInputs)
{
	m_ui->setupUi(this);
	setModal(true);

	m_ui->usernameEdit->setText(user.name);

	m_ui->passwordLabel->setVisible(m_showPasswordInputs);
	m_ui->passwordLineEdit->setVisible(m_showPasswordInputs);

	for (const Core::Client& client : clients)
	{
		m_ui->clientsComboBox->addItem(client.name, client.id);
	}

	m_ui->adminCheckBox->setChecked(user.admin);

	connect(m_ui->clientsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &UserEditorDialog::onClientChanged);

	if (!user.admin)
	{
		const auto it = std::find_if(clients.begin(), clients.end(),
			[&user](const Core::Client& client) { return user.clientId == client.id; });

		// "reset" index, so currentIndexChanged will be emitted after setCurrentIndex
		m_ui->clientsComboBox->setCurrentIndex(-1);

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

void UserEditorDialog::onClientChanged(int clientIndex)
{
	m_ui->groupsListWidget->clear();

	if (clientIndex == -1)
	{
		return;
	}

	m_ui->groupsListWidget->setItems(m_clients[clientIndex].groups);
	m_ui->groupsListWidget->setCheckedItems(m_user.groups);
}

void UserEditorDialog::saveChanges()
{
	const QString username = m_ui->usernameEdit->text().trimmed();
	if (username.isEmpty())
	{
		QMessageBox::warning(this, "Ошибка валидации", "Необходимо ввести имя пользователя.");
		return;
	}

	if (!m_uniquenessValidator(username))
	{
		QMessageBox::warning(this, "Ошибка валидации", "Имя пользователя должно быть уникальным.");
		return;
	}

	if (m_showPasswordInputs)
	{
		const QString password = m_ui->passwordLineEdit->text().trimmed();
		if (password.isEmpty())
		{
			QMessageBox::warning(this, "Ошибка валидации", "Необходимо ввести пароль.");
			return;
		}
	}

	const bool admin = m_ui->adminCheckBox->isChecked();

	Core::User user = Core::User(username, admin);
	user.banned = m_user.banned;

	if (m_showPasswordInputs)
	{
		const QString password = m_ui->passwordLineEdit->text().trimmed();
		user.password = password;
	}

	if (!user.admin)
	{
		const int clientIndex = m_ui->clientsComboBox->currentIndex();
		if (clientIndex == -1)
		{
			QMessageBox::warning(this, "Ошибка валидации", "Необходимо выбрать клиента или сделать пользователя администратором.");
			return;
		}

		const QString clientId = m_clients[clientIndex].id;
		user.clientId = clientId;

		QList<QString> checkedGroups = m_ui->groupsListWidget->checkedItems();
		if (checkedGroups.isEmpty())
		{
			QMessageBox::warning(this, "Ошибка валидации", "Необходимо выбрать хотя бы одну группу или сделать пользователя администратором.");
			return;
		}

		user.groups = checkedGroups;
	}

	emit userChanged(user);

	accept();
}

void UserEditorDialog::discardChanges()
{
	reject();
}
