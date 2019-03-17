#include "usereditordialog.h"
#include "ui_usereditordialog.h"
#include <QPushButton>
#include <QMessageBox>

using namespace Core;

namespace
{

const QVector<User::Role> c_roles =
{
	User::Role::ClientUser,
	User::Role::ClientGroupSupervisor,
	User::Role::ClientSupervisor,
	User::Role::Admin
};

QString roleToString(const User::Role& role)
{
	switch (role)
	{
	case User::Role::Admin: return "Администратор";
	case User::Role::ClientUser: return "Пользователь";
	case User::Role::ClientGroupSupervisor: return "Супервизор групп";
	case User::Role::ClientSupervisor: return "Супервизор клиента";
	default: return QString("Неизвестная роль - %1").arg(static_cast<int>(role));
	}
}

QString roleDescription(const User::Role& role)
{
	switch (role)
	{
	case User::Role::Admin:
		return "Администратор всего продукта.\n"
			"Имеет доступ к диалогам всех клиентов.\n"
			"Может просматривать статистику по любому пользователю и диалогу.\n"
			"Может пользоваться Редактором Диалогов.";
	case User::Role::ClientUser:
		return "Пользователь клиента.\n"
			"Имеет доступ к диалогам выбранных групп клиента.\n"
			"Может просматривать только свою статистику.\n"
			"Не может пользоваться Редактором Диалогов";
	case User::Role::ClientGroupSupervisor:
		return "Супервизор групп.\n"
			"Имеет доступ к диалогам выбранных групп клиента.\n"
			"Может просматривать статистику по любому пользователю и диалогу выбранных групп.\n"
			"Не может пользоваться Редактором Диалогов";
	case User::Role::ClientSupervisor:
		return "Супервизор клиента.\n"
			"Имеет доступ к диалогам выбранного клиента.\n"
			"Может просматривать статистику по любому пользователю и диалогу выбранного клиента.\n"
			"Не может пользоваться Редактором Диалогов";
	default:
		return QString("Неизвестная роль - %1").arg(static_cast<int>(role));
	}
}

}

UserEditorDialog::UserEditorDialog(const User& user, const UniquenessValidator& validator, const QList<Client>& clients,
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

	for (const Client& client : clients)
	{
		m_ui->clientsComboBox->addItem(client.name, client.id);
	}

	connect(m_ui->roleComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &UserEditorDialog::onRoleChanged);

	for (const User::Role& role : c_roles)
	{
		m_ui->roleComboBox->addItem(roleToString(role), static_cast<int>(role));

		if (user.role == role)
		{
			m_ui->roleComboBox->setCurrentIndex(m_ui->roleComboBox->count() - 1);
		}
	}

	QIcon infoIcon = style()->standardIcon(QStyle::SP_MessageBoxInformation);
	QPixmap infoPixmap = infoIcon.pixmap(QSize(16, 16));
	m_ui->roleInfoLabel->setPixmap(infoPixmap);

	connect(m_ui->clientsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &UserEditorDialog::onClientChanged);

	if (user.role != User::Role::Admin)
	{
		const auto it = std::find_if(clients.begin(), clients.end(),
			[&user](const Client& client) { return user.clientId == client.id; });

		// "reset" index, so currentIndexChanged will be emitted after setCurrentIndex
		m_ui->clientsComboBox->setCurrentIndex(-1);

		const int index = it == clients.end() ? 0 : std::distance(clients.begin(), it);
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

void UserEditorDialog::onRoleChanged(int roleIndex)
{
	if (roleIndex == -1)
	{
		return;
	}

	const User::Role& role = c_roles[roleIndex];

	m_ui->roleInfoLabel->setToolTip(roleDescription(role));

	m_ui->clientsComboBox->setEnabled(role == User::Role::ClientUser || role == User::Role::ClientGroupSupervisor || role == User::Role::ClientSupervisor);

	m_ui->groupsListWidget->setEnabled(role == User::Role::ClientUser || role == User::Role::ClientGroupSupervisor);
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

	User user;
	user.role = static_cast<User::Role>(m_ui->roleComboBox->currentData().toInt());
	user.name = username;
	user.banned = m_user.banned;

	if (m_showPasswordInputs)
	{
		const QString password = m_ui->passwordLineEdit->text().trimmed();
		user.password = password;
	}

	if (user.role == User::Role::ClientUser || user.role == User::Role::ClientGroupSupervisor || user.role == User::Role::ClientSupervisor)
	{
		const int clientIndex = m_ui->clientsComboBox->currentIndex();
		if (clientIndex == -1)
		{
			QMessageBox::warning(this, "Ошибка валидации", "Необходимо выбрать клиента или сделать пользователя администратором.");
			return;
		}

		const QString clientId = m_clients[clientIndex].id;
		user.clientId = clientId;

		if (user.role == User::Role::ClientUser || user.role == User::Role::ClientGroupSupervisor)
		{
			QList<QString> checkedGroups = m_ui->groupsListWidget->checkedItems();
			if (checkedGroups.isEmpty())
			{
				QMessageBox::warning(this, "Ошибка валидации", "Необходимо выбрать хотя бы одну группу или сделать пользователя администратором.");
				return;
			}

			user.groups = checkedGroups;
		}
	}

	emit userChanged(user);

	accept();
}

void UserEditorDialog::discardChanges()
{
	reject();
}
