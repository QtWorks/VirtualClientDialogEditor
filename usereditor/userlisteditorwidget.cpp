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

QStringList UserListEditorWidget::items() const
{
	QStringList result;

	for (const UserListDataModel::Index& index : m_model.indexes())
	{
		result << m_model.get(index).name;
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
		const UserListDataModel::Index index = m_model.findIndex([&username](const Core::User& user){ return user.name == username; });
		removingUsers.append(m_model.get(index));
	}

	m_backendConnection->updateUsers({ {}, removingUsers, {} });
}

void UserListEditorWidget::onItemEditRequested(const QString& username)
{
	const UserListDataModel::Index index = m_model.findIndex(
		[&username](const Core::User& user){ return user.name == username; });
	Q_ASSERT(index != -1);

	UserEditorDialog* editorWindow = new UserEditorDialog(m_model.get(index), this);
	connect(editorWindow, &UserEditorDialog::userChanged, [this, index, username](Core::User user)
	{
		const Core::User& sourceUser = m_model.get(index);
		if (sourceUser == m_model.get(index))
		{
			return;
		}

		showProgressDialog("Изменение данных", "Идет изменение данных. Пожалуйста, подождите.");

		const QMap<Core::User, Core::User> updated = {
			{ sourceUser, user }
		};
		m_backendConnection->updateUsers({ updated, {}, {} });
	});

	editorWindow->show();
}

void UserListEditorWidget::onItemCreateRequested()
{
	const Core::User user = { };

	UserEditorDialog* editorWindow = new UserEditorDialog(user, this);
	connect(editorWindow, &UserEditorDialog::userChanged, [this](Core::User user)
	{
		showProgressDialog("Добавление данных", "Идет добавление данных. Пожалуйста, подождите.");
		m_backendConnection->updateUsers({ {}, {}, { user } });
	});

	editorWindow->show();
}

void UserListEditorWidget::onUsersLoaded(Core::IBackendConnection::QueryId /*queryId*/, const QList<Core::User>& users)
{
	m_model.setData(users);

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
