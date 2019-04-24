#include "groupslisteditorwidget.h"
#include "groupeditordialog.h"
#include "ui_listeditorwidget.h"

#include <QMessageBox>
#include <QPushButton>

using namespace Core;

namespace
{

QString toLowerCase(const QString& str)
{
	return str.left(1).toLower() + str.mid(1);
}

void removeGroups(Client& client, const QStringList& groups)
{
	for (const QString& removingGroup : groups)
	{
		client.groups.erase(
			std::remove_if(client.groups.begin(), client.groups.end(), [&removingGroup](const Group& group) { return group.name == removingGroup; }),
			client.groups.end());
	}
}

}

GroupsListEditorWidget::GroupsListEditorWidget(IBackendConnectionSharedPtr backendConnection, QWidget* parent)
	: ListEditorWidget(parent)
	, m_backendConnection(backendConnection)
	, m_updating(false)
{
	connect(this, &ListEditorWidget::itemEditRequested, this, &GroupsListEditorWidget::onItemEditRequested);
	connect(this, &ListEditorWidget::itemCreateRequested, this, &GroupsListEditorWidget::onItemCreateRequested);

	connect(m_backendConnection.get(), &IBackendConnection::clientsLoaded, this, &GroupsListEditorWidget::onClientsLoaded);
	connect(m_backendConnection.get(), &IBackendConnection::clientsLoadFailed, this, &GroupsListEditorWidget::onClientsLoadFailed);
	connect(m_backendConnection.get(), &IBackendConnection::clientsUpdated, this, &GroupsListEditorWidget::onClientsUpdated);
	connect(m_backendConnection.get(), &IBackendConnection::clientsUpdateFailed, this, &GroupsListEditorWidget::onClientsUpdateFailed);
	connect(m_backendConnection.get(), &IBackendConnection::statisticsCleanupSuccess, this, &GroupsListEditorWidget::onCleanupStatisticsSuccess);
	connect(m_backendConnection.get(), &IBackendConnection::statisticsCleanupFailure, this, &GroupsListEditorWidget::onCleanupStatisticsFailure);

	m_ui->additionalButtonsWidget->show();

	connect(m_ui->cleanupStatisticsButton, &QPushButton::clicked, this, &GroupsListEditorWidget::cleanupStatistics);
	connect(m_ui->banButton, &QPushButton::clicked, this, &GroupsListEditorWidget::processBanSelected);

	connect(m_ui->listWidget, &QListWidget::itemSelectionChanged, this, &GroupsListEditorWidget::onGroupSelectionChanged);
	onGroupSelectionChanged();
}

void GroupsListEditorWidget::loadData()
{
	showProgressDialog("Загрузка данных", "Идет загрузка данных. Пожалуйста, подождите.");

	m_backendConnection->loadClients();
}

void GroupsListEditorWidget::setCurrentClient(const Client& client)
{
	m_currentClient = client;
	updateData();
}

QStringList GroupsListEditorWidget::items() const
{
	QStringList result;

	for (const Group& group : m_currentClient.groups)
	{
		result << group.name;
	}

	return result;
}

void GroupsListEditorWidget::removeItems(const QStringList& items)
{
	showProgressDialog("Удаление данных", "Идет удаление данных. Пожалуйста, подождите.");

	m_updating = true;

	Client updatedClient = m_currentClient;
	removeGroups(updatedClient, items);

	m_backendConnection->updateClients({ { { m_currentClient, updatedClient } }, {}, {} });
}

void GroupsListEditorWidget::onItemEditRequested(const QString& groupName)
{
	const auto it = std::find_if(m_currentClient.groups.begin(), m_currentClient.groups.end(),
		[&groupName](const Core::Group& group){ return group.name == groupName; });
	Q_ASSERT(it != m_currentClient.groups.end());
	const int index = std::distance(m_currentClient.groups.begin(), it);

	const auto groupNameValidator = [this, index](const QString& name)
	{
		for (int i = 0; i < m_currentClient.groups.length(); i++)
		{
			if (i == index)
			{
				continue;
			}

			if (m_currentClient.groups[i].name.compare(name, Qt::CaseInsensitive) == 0)
			{
				return false;
			}
		}

		return true;
	};

	GroupEditorDialog* editorWindow = new GroupEditorDialog(*it, groupNameValidator);
	connect(editorWindow, &GroupEditorDialog::groupChanged,
		[this, index](Core::Group group) { updateGroup(index, group); });

	editorWindow->show();
}

void GroupsListEditorWidget::onItemCreateRequested()
{
	const Core::Group group = { "", "", false };

	const auto groupNameValidator = [this](const QString& name)
	{
		for (int i = 0; i < m_currentClient.groups.length(); i++)
		{
			if (m_currentClient.groups[i].name.compare(name, Qt::CaseInsensitive) == 0)
			{
				return false;
			}
		}

		return true;
	};

	GroupEditorDialog* editorWindow = new GroupEditorDialog(group, groupNameValidator);
	connect(editorWindow, &GroupEditorDialog::groupChanged,
		[this](Core::Group group) { addGroup(group); });

	editorWindow->show();
}

void GroupsListEditorWidget::onClientsLoaded(IBackendConnection::QueryId /*queryId*/, const QList<Client>& clients)
{
	m_clients = clients;

	updateData();

	hideProgressDialog();

	if (m_updating)
	{
		QMessageBox::information(this, "Сохранение данных", "Сохранение данных завершилось успешно.");
		m_updating = false;
	}
}

void GroupsListEditorWidget::onClientsLoadFailed(IBackendConnection::QueryId /*queryId*/, const QString& error)
{
	hideProgressDialog();

	QMessageBox::warning(this, "Загрузка данных", "Загрузка данных завершилась ошибкой: " + toLowerCase(error) + ".");
}

void GroupsListEditorWidget::onClientsUpdated(IBackendConnection::QueryId /*queryId*/)
{
	hideProgressDialog();

	loadData();
}

void GroupsListEditorWidget::onClientsUpdateFailed(IBackendConnection::QueryId /*queryId*/, const QString& error)
{
	hideProgressDialog();

	QMessageBox::warning(this, "Сохранение данных", "Сохранение данных завершилось ошибкой: " + toLowerCase(error) + ".");
}

void GroupsListEditorWidget::onCleanupStatisticsSuccess(Core::IBackendConnection::QueryId queryId)
{
	if (queryId != m_cleanupStatisticsQueryId)
	{
		return;
	}

	hideProgressDialog();

	QMessageBox::information(this, "Очистка статистики", "Статистика успешно очищена.");
}

void GroupsListEditorWidget::onCleanupStatisticsFailure(Core::IBackendConnection::QueryId queryId, const QString& error)
{
	if (queryId != m_cleanupStatisticsQueryId)
	{
		return;
	}

	hideProgressDialog();

	QMessageBox::warning(this, "Очистка статистики", "Очистка статистики завершилось ошибкой: " + toLowerCase(error) + ".");
}

void GroupsListEditorWidget::updateGroup(int index, const Core::Group& group)
{
	const Core::Group& sourceGroup = m_currentClient.groups[index];
	if (group == sourceGroup)
	{
		return;
	}

	showProgressDialog("Изменение данных", "Идет изменение данных. Пожалуйста, подождите.");

	Client updatedClient = m_currentClient;
	updatedClient.groups[index] = group;
	m_backendConnection->updateClients({ { { m_currentClient, updatedClient }  }, {}, {} });
}

void GroupsListEditorWidget::addGroup(const Core::Group& group)
{
	showProgressDialog("Добавление данных", "Идет добавление данных. Пожалуйста, подождите.");

	Client updatedClient = m_currentClient;
	updatedClient.groups.append(group);
	m_backendConnection->updateClients({ { { m_currentClient, updatedClient } }, {}, {} });
}

void GroupsListEditorWidget::onGroupSelectionChanged()
{
	const QList<QListWidgetItem*> selectedItems = m_ui->listWidget->selectedItems();
	m_ui->cleanupStatisticsButton->setEnabled(selectedItems.size() == 1);

	m_ui->banButton->setEnabled(selectedItems.size() == 1);

	const bool isBanned = selectedItems.size() == 1 && m_currentClient.groups[m_ui->listWidget->currentRow()].banned;
	m_ui->banButton->setText(isBanned ? "Разблокировать" : "Заблокировать");
}

void GroupsListEditorWidget::cleanupStatistics()
{
	const QList<QListWidgetItem*> selectedItems = m_ui->listWidget->selectedItems();
	Q_ASSERT(selectedItems.size() == 1);
	const QString groupName = selectedItems.first()->text();

	QMessageBox messageBox(QMessageBox::Question,
		"Очистить статистику",
		"Вы действительно хотите очистить статистику группы \"" + groupName + "\"?",
		QMessageBox::Yes | QMessageBox::No,
		this);
	messageBox.setButtonText(QMessageBox::Yes, tr("Да"));
	messageBox.setButtonText(QMessageBox::No, tr("Нет"));

	const int answer = messageBox.exec();
	if (answer == QMessageBox::Yes)
	{
		const auto it = std::find_if(m_currentClient.groups.begin(), m_currentClient.groups.end(),
			[&groupName](const Core::Group& group){ return group.name == groupName; });
		Q_ASSERT(it != m_currentClient.groups.end());

		m_cleanupStatisticsQueryId = m_backendConnection->cleanupGroupStatistics(m_currentClient.databaseName, it->id);
	}
}

void GroupsListEditorWidget::processBanSelected()
{
	const int selectedIndex = m_ui->listWidget->currentRow();
	Q_ASSERT(selectedIndex >= 0 && selectedIndex <= m_currentClient.groups.size());

	Core::Group group = m_currentClient.groups[selectedIndex];
	group.banned = !group.banned;
	updateGroup(selectedIndex, group);
}
