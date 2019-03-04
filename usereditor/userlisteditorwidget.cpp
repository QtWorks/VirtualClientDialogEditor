#include "userlisteditorwidget.h"
#include "usereditordialog.h"

#include <QMessageBox>

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
