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

void UserListEditorWidget::setClients(const QList<Core::Client>& clients)
{
	m_clients = clients;
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
	const Core::User& sourceUser = m_model[index];
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
	showProgressDialog("Добавление данных", "Идет добавление данных. Пожалуйста, подождите.");
	m_backendConnection->updateUsers({ {}, {}, { user } });
}

void UserListEditorWidget::onItemEditRequested(const QString& username)
{
	const auto it = std::find_if(m_model.begin(), m_model.end(),
		[&username](const Core::User& user){ return user.name == username; });
	Q_ASSERT(it != m_model.end());
	const int index = std::distance(m_model.begin(), it);

	const auto validator = [this, index](const QString& name)
	{
		for (int i = 0; i < m_model.length(); i++)
		{
			if (i == index)
			{
				continue;
			}

			const Core::User& user = m_model[i];
			if (user.name.compare(name, Qt::CaseInsensitive) == 0)
			{
				return false;
			}
		}

		return true;
	};

	UserEditorDialog* editorWindow = new UserEditorDialog(*it, validator, m_clients, this);
	connect(editorWindow, &UserEditorDialog::userChanged, [this, index](Core::User user) { updateUser(index, user); });

	editorWindow->show();
}

void UserListEditorWidget::onItemCreateRequested()
{
	const Core::User user = { };

	const auto validator = [this](const QString& name)
	{
		for (int i = 0; i < m_model.length(); i++)
		{
			const Core::User& user = m_model[i];
			if (user.name.compare(name, Qt::CaseInsensitive) == 0)
			{
				return false;
			}
		}

		return true;
	};

	UserEditorDialog* editorWindow = new UserEditorDialog(user, validator, m_clients, this);
	connect(editorWindow, &UserEditorDialog::userChanged, [this](Core::User user) { addUser(user); });

	editorWindow->show();
}

void UserListEditorWidget::onUsersLoaded(Core::IBackendConnection::QueryId /*queryId*/, const QList<Core::User>& users)
{
	m_model = users;

	updateData();

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
