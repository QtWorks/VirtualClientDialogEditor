#include "userlisteditorwidget.h"
#include "usereditordialog.h"

#include "ui_listeditorwidget.h"

#include <QMessageBox>
#include <QPushButton>

namespace
{

QString toLowerCase(const QString& str)
{
	return str.left(1).toLower() + str.mid(1);
}

}

UserListEditorWidget::UserListEditorWidget(IBackendConnectionSharedPtr backendConnection, QWidget* parent)
	: ListEditorWidget(parent)
	, m_backendConnection(backendConnection)
	, m_updating(false)
{
	connect(this, &ListEditorWidget::itemEditRequested, this, &UserListEditorWidget::onItemEditRequested);
	connect(this, &ListEditorWidget::itemCreateRequested, this, &UserListEditorWidget::onItemCreateRequested);

	connect(m_backendConnection.get(), &Core::IBackendConnection::usersLoaded, this, &UserListEditorWidget::onUsersLoaded);
	connect(m_backendConnection.get(), &Core::IBackendConnection::usersLoadFailed, this, &UserListEditorWidget::onUsersLoadFailed);
	connect(m_backendConnection.get(), &Core::IBackendConnection::usersUpdated, this, &UserListEditorWidget::onUsersUpdated);
	connect(m_backendConnection.get(), &Core::IBackendConnection::usersUpdateFailed, this, &UserListEditorWidget::onUsersUpdateFailed);
	connect(m_backendConnection.get(), &Core::IBackendConnection::statisticsCleanupSuccess, this, &UserListEditorWidget::onCleanupStatisticsSuccess);
	connect(m_backendConnection.get(), &Core::IBackendConnection::statisticsCleanupFailure, this, &UserListEditorWidget::onCleanupStatisticsFailure);

	m_ui->additionalButtonsWidget->show();

	connect(m_ui->cleanupStatisticsButton, &QPushButton::clicked, this, &UserListEditorWidget::cleanupStatistics);
	connect(m_ui->banButton, &QPushButton::clicked, this, &UserListEditorWidget::processBanSelected);

	connect(m_ui->listWidget, &QListWidget::itemSelectionChanged, this, &UserListEditorWidget::onUserSelectionChanged);
	onUserSelectionChanged();
}

void UserListEditorWidget::loadData()
{
	showProgressDialog("Загрузка данных", "Идет загрузка данных. Пожалуйста, подождите.");

	m_backendConnection->loadUsers();
}

QList<Core::User> UserListEditorWidget::currentUsers() const
{
	return m_model;
}

QList<Core::User> UserListEditorWidget::allUsers() const
{
	return m_originalModel;
}

void UserListEditorWidget::addUsers(const QList<Core::User>& users)
{
	showProgressDialog("Добавление данных", "Идет добавление данных. Пожалуйста, подождите.");

	m_backendConnection->updateUsers({ {}, {}, users });
}

void UserListEditorWidget::setClients(const QList<Core::Client>& clients)
{
	m_clients = clients;
}

void UserListEditorWidget::setClientFilter(const QString& client)
{
	m_currentClient = client;

	m_model.clear();

	for (const Core::User& user : m_originalModel)
	{
		if (user.clientId == m_currentClient)
		{
			m_model.append(user);
		}
	}

	qSort(m_model);

	updateData();
}

void UserListEditorWidget::resetClientFilter()
{
	m_model = m_originalModel;
	qSort(m_model);
	updateData();
}

QStringList UserListEditorWidget::items() const
{
	QStringList result;

	for (const Core::User& user : m_model)
	{
		result << user.name;
	}

	return result;
}

void UserListEditorWidget::removeItems(const QStringList& usernames)
{
	showProgressDialog("Удаление данных", "Идет удаление данных. Пожалуйста, подождите.");

	m_updating = true;

	QList<Core::User> removingUsers;
	for (const QString& username : usernames)
	{
		const auto it = std::find_if(m_model.begin(), m_model.end(),
			[&username](const Core::User& user){ return user.name == username; });
		Q_ASSERT(it != m_model.end());
		removingUsers.append(*it);
	}

	m_backendConnection->updateUsers({ {}, removingUsers, {} });
}

void UserListEditorWidget::updateUser(int index, const Core::User& user)
{
	const Core::User& sourceUser = m_originalModel[index];
	if (sourceUser == user)
	{
		return;
	}

	showProgressDialog("Изменение данных", "Идет изменение данных. Пожалуйста, подождите.");

	const QMap<Core::User, Core::User> updated = {
		{ sourceUser, user }
	};
	m_backendConnection->updateUsers({ updated, {}, {} });
}

void UserListEditorWidget::addUser(const Core::User& user)
{
	addUsers({ user });
}

void UserListEditorWidget::onItemEditRequested(const QString& username)
{
	const auto it = std::find_if(m_originalModel.begin(), m_originalModel.end(),
		[&username](const Core::User& user){ return user.name == username; });
	Q_ASSERT(it != m_originalModel.end());
	const int index = std::distance(m_originalModel.begin(), it);

	const auto validator = [this, index](const QString& name)
	{
		for (int i = 0; i < m_originalModel.length(); i++)
		{
			if (i == index)
			{
				continue;
			}

			const Core::User& user = m_originalModel[i];
			if (user.name.compare(name, Qt::CaseInsensitive) == 0)
			{
				return false;
			}
		}

		return true;
	};

	UserEditorDialog* editorWindow = new UserEditorDialog(*it, validator, m_clients, false, this);
	connect(editorWindow, &UserEditorDialog::userChanged, [this, index](Core::User user) { updateUser(index, user); });

	editorWindow->show();
}

void UserListEditorWidget::onItemCreateRequested()
{
	const Core::User user = { };

	const auto validator = [this](const QString& name)
	{
		for (const Core::User& user : m_originalModel)
		{
			if (user.name.compare(name, Qt::CaseInsensitive) == 0)
			{
				return false;
			}
		}

		return true;
	};

	UserEditorDialog* editorWindow = new UserEditorDialog(user, validator, m_clients, true, this);
	connect(editorWindow, &UserEditorDialog::userChanged, [this](Core::User user) { addUser(user); });

	editorWindow->show();
}

void UserListEditorWidget::onUsersLoaded(Core::IBackendConnection::QueryId /*queryId*/, const QList<Core::User>& users)
{
	m_originalModel = users;
	setClientFilter(m_currentClient);

	hideProgressDialog();

	if (m_updating)
	{
		QMessageBox::information(this, "Сохранение данных", "Сохранение данных завершилось успешно.");
		m_updating = false;
	}
}

void UserListEditorWidget::onUsersLoadFailed(Core::IBackendConnection::QueryId /*queryId*/, const QString& error)
{
	hideProgressDialog();

	QMessageBox::warning(this, "Загрузка данных", "Загрузка данных завершилась ошибкой: " + toLowerCase(error) + ".");
}

void UserListEditorWidget::onUsersUpdated(Core::IBackendConnection::QueryId /*queryId*/)
{
	hideProgressDialog();

	loadData();
}

void UserListEditorWidget::onUsersUpdateFailed(Core::IBackendConnection::QueryId /*queryId*/, const QString& error)
{
	hideProgressDialog();

	QMessageBox::warning(this, "Сохранение данных", "Сохранение данных завершилось ошибкой: " + toLowerCase(error) + ".");
}

void UserListEditorWidget::onCleanupStatisticsSuccess(Core::IBackendConnection::QueryId queryId)
{
	if (queryId != m_cleanupStatisticsQueryId)
	{
		return;
	}

	hideProgressDialog();

	QMessageBox::information(this, "Очистка статистики", "Статистика успешно очищена.");
}

void UserListEditorWidget::onCleanupStatisticsFailure(Core::IBackendConnection::QueryId queryId, const QString& error)
{
	if (queryId != m_cleanupStatisticsQueryId)
	{
		return;
	}

	hideProgressDialog();

	QMessageBox::warning(this, "Очистка статистики", "Очистка статистики завершилось ошибкой: " + toLowerCase(error) + ".");
}

void UserListEditorWidget::onUserSelectionChanged()
{
	const QList<QListWidgetItem*> selectedItems = m_ui->listWidget->selectedItems();

	m_ui->banButton->setEnabled(selectedItems.size() == 1);

	if (selectedItems.size() != 1)
	{
		m_ui->cleanupStatisticsButton->setEnabled(false);
		return;
	}

	const QString username = selectedItems.first()->text();
	const auto it = std::find_if(m_originalModel.begin(), m_originalModel.end(),
		[&username](const Core::User& user){ return user.name == username; });
	const Core::User& user = *it;

	m_ui->cleanupStatisticsButton->setEnabled(user.role != Core::User::Role::Admin);

	m_ui->banButton->setEnabled(user.role != Core::User::Role::Admin);
	m_ui->banButton->setText(user.banned ? "Разблокировать" : "Заблокировать");
}

void UserListEditorWidget::cleanupStatistics()
{
	const auto it = std::find_if(m_clients.begin(), m_clients.end(),
		[this](const Core::Client& client){ return client.id == m_currentClient; });
	Q_ASSERT(it != m_clients.end());
	const QString& clientId = it->databaseName;

	const QList<QListWidgetItem*> selectedItems = m_ui->listWidget->selectedItems();
	Q_ASSERT(selectedItems.size() == 1);
	const QString username = selectedItems.first()->text();

	QMessageBox messageBox(QMessageBox::Question,
		"Очистить статистику",
		"Вы действительно хотите очистить статистику пользователя \"" + username + "\"?",
		QMessageBox::Yes | QMessageBox::No,
		this);
	messageBox.setButtonText(QMessageBox::Yes, tr("Да"));
	messageBox.setButtonText(QMessageBox::No, tr("Нет"));

	const int answer = messageBox.exec();
	if (answer == QMessageBox::Yes)
	{
		m_cleanupStatisticsQueryId = m_backendConnection->cleanupUserStatistics(clientId, username);
	}
}

void UserListEditorWidget::processBanSelected()
{
	const QList<QListWidgetItem*> selectedItems = m_ui->listWidget->selectedItems();
	const QString username = selectedItems.first()->text();
	const auto it = std::find_if(m_originalModel.begin(), m_originalModel.end(),
		[&username](const Core::User& user){ return user.name == username; });
	const int userIndex = std::distance(m_originalModel.begin(), it);

	Core::User user = m_originalModel[userIndex];
	user.banned = !user.banned;
	updateUser(userIndex, user);
}
