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
	connect(this, &ListEditorWidget::itemsRemoveRequested, this, &UserListEditorWidget::onItemsRemoveRequested);
	connect(this, &ListEditorWidget::itemCreateRequested, this, &UserListEditorWidget::onItemCreateRequested);

	connect(&m_model, &IListDataModel::diffRecordsCountChanged, this, &UserListEditorWidget::onDiffRecordsCountChanged);
	onDiffRecordsCountChanged(0);

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

bool UserListEditorWidget::itemHasChanges(const QString& username) const
{
	const UserListDataModel::Index index = m_model.findIndex([&username](const Core::User& user){ return user.name == username; });
	return m_model.hasChanges(index);
}

bool UserListEditorWidget::itemIsAdded(const QString& item) const
{
	const QList<Core::User> addedItems = m_model.added();
	return std::find_if(addedItems.begin(), addedItems.end(),
		[&item](const Core::User& user) { return user.name == item; }) != addedItems.end();
}

void UserListEditorWidget::saveChanges()
{
	showProgressDialog("Сохранение данных", "Идет сохранение данных. Пожалуйста, подождите.");

	m_updating = true;
	m_backendConnection->updateUsers({ m_model.updated(), m_model.deleted(), m_model.added() });
}

void UserListEditorWidget::revertChanges(const QString& username)
{
	const UserListDataModel::Index index = m_model.findIndex([&username](const Core::User& user){ return user.name == username; });
	return m_model.revert(index);
}

void UserListEditorWidget::revertAllChanges()
{
	m_model.revertAll();
}

void UserListEditorWidget::onItemEditRequested(const QString& username)
{
	const UserListDataModel::Index index = m_model.findIndex([&username](const Core::User& user){ return user.name == username; });
	Q_ASSERT(index != -1);

	UserEditorDialog* dialog = new UserEditorDialog(m_model.get(index), this);
	connect(dialog, &UserEditorDialog::userChanged, [this, index, username](Core::User user)
	{
		if (user != m_model.get(index))
		{
			m_model.update(index, user);
			updateItem(username, user.name);
		}
	});

	dialog->show();
}

void UserListEditorWidget::onItemsRemoveRequested(const QStringList& users)
{
	QMessageBox messageBox(QMessageBox::Question,
		"Удаление пользователей",
		"Вы действительно хотите удалить " + QString(users.size() > 1 ? "выбранных пользователей" : "выбранного пользователя ") + "?",
		QMessageBox::Yes | QMessageBox::No,
		this);
	messageBox.setButtonText(QMessageBox::Yes, tr("Да"));
	messageBox.setButtonText(QMessageBox::No, tr("Нет"));

	const int answer = messageBox.exec();
	if (answer != QMessageBox::Yes)
	{
		return;
	}

	for (const QString& username : users)
	{
		const UserListDataModel::Index index = m_model.findIndex([&username](const Core::User& user) { return user.name == username; });
		Q_ASSERT(index != -1);

		removeItem(username);

		m_model.remove(index);
	}
}

void UserListEditorWidget::onItemCreateRequested()
{
	const Core::User user = { };
	UserEditorDialog* dialog = new UserEditorDialog(user, this);

	connect(dialog, &UserEditorDialog::userChanged, [this](Core::User user)
	{
		m_model.append(user);

		addItem(user.name);
	});

	dialog->show();
}

void UserListEditorWidget::onDiffRecordsCountChanged(int count)
{
	m_saveButton->setEnabled(count > 0);
	m_revertAllButton->setEnabled(count > 0);
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
