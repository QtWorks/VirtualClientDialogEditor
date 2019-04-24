#include "clientlisteditorwidget.h"
#include "clienteditordialog.h"
#include "core/ibackendconnection.h"
#include "ui_listeditorwidget.h"

#include "logger.h"

#include <QMessageBox>
#include <QPushButton>

namespace
{

QString toLowerCase(const QString& str)
{
	return str.left(1).toLower() + str.mid(1);
}

}

ClientListEditorWidget::ClientListEditorWidget(IBackendConnectionSharedPtr backendConnection, QWidget* parent)
	: ListEditorWidget(parent)
	, m_backendConnection(backendConnection)
	, m_updating(false)
{
	connect(this, &ListEditorWidget::itemEditRequested, this, &ClientListEditorWidget::onItemEditRequested);
	connect(this, &ListEditorWidget::itemCreateRequested, this, &ClientListEditorWidget::onItemCreateRequested);

	connect(m_backendConnection.get(), &Core::IBackendConnection::clientsLoaded, this, &ClientListEditorWidget::onClientsLoaded);
	connect(m_backendConnection.get(), &Core::IBackendConnection::clientsLoadFailed, this, &ClientListEditorWidget::onClientsLoadFailed);
	connect(m_backendConnection.get(), &Core::IBackendConnection::clientsUpdated, this, &ClientListEditorWidget::onClientsUpdated);
	connect(m_backendConnection.get(), &Core::IBackendConnection::clientsUpdateFailed, this, &ClientListEditorWidget::onClientsUpdateFailed);
	connect(m_backendConnection.get(), &Core::IBackendConnection::statisticsCleanupSuccess, this, &ClientListEditorWidget::onCleanupStatisticsSuccess);
	connect(m_backendConnection.get(), &Core::IBackendConnection::statisticsCleanupFailure, this, &ClientListEditorWidget::onCleanupStatisticsFailure);

	m_ui->additionalButtonsWidget->show();

	connect(m_ui->cleanupStatisticsButton, &QPushButton::clicked, this, &ClientListEditorWidget::cleanupStatistics);
	connect(m_ui->banButton, &QPushButton::clicked, this, &ClientListEditorWidget::processBanSelected);

	connect(m_ui->listWidget, &QListWidget::itemSelectionChanged, this, &ClientListEditorWidget::onClientSelectionChanged);
	onClientSelectionChanged();
}

void ClientListEditorWidget::loadData()
{
	showProgressDialog("Загрузка данных", "Идет загрузка данных. Пожалуйста, подождите.");

	m_backendConnection->loadClients();
}

QStringList ClientListEditorWidget::items() const
{
	QStringList result;

	for (const Core::Client& client : m_model)
	{
		result << client.name;
	}

	return result;
}

void ClientListEditorWidget::removeItems(const QStringList& clients)
{
	showProgressDialog("Удаление данных", "Идет удаление данных. Пожалуйста, подождите.");

	m_updating = true;

	QList<Core::Client> removingClients;
	for (const QString& clientName : clients)
	{
		const auto it = std::find_if(m_model.begin(), m_model.end(),
			[&clientName](const Core::Client& client){ return client.name == clientName; });
		Q_ASSERT(it != m_model.end());
		removingClients.append(*it);
	}

	m_backendConnection->updateClients({ {}, removingClients, {} });
}

void ClientListEditorWidget::onItemEditRequested(const QString& clientName)
{
	const auto it = std::find_if(m_model.begin(), m_model.end(),
		[&clientName](const Core::Client& client){ return client.name == clientName; });
	Q_ASSERT(it != m_model.end());
	const int index = std::distance(m_model.begin(), it);

	const auto nameValidator = [this, index](const QString& name)
	{
		for (int i = 0; i < m_model.length(); i++)
		{
			if (i == index)
			{
				continue;
			}

			if (m_model[i].name.compare(name, Qt::CaseInsensitive) == 0)
			{
				return false;
			}
		}

		return true;
	};

	const auto databaseNameValidator = [this, index](const QString& name)
	{
		for (int i = 0; i < m_model.length(); i++)
		{
			if (i == index)
			{
				continue;
			}

			if (m_model[i].databaseName.compare(name, Qt::CaseInsensitive) == 0)
			{
				return false;
			}
		}

		return true;
	};

	ClientEditorDialog* editorWindow = new ClientEditorDialog(*it, nameValidator, databaseNameValidator);
	connect(editorWindow, &ClientEditorDialog::clientChanged,
		[this, index](Core::Client client) { updateClient(index, client); });

	editorWindow->show();
}

void ClientListEditorWidget::onItemCreateRequested()
{
	const Core::Client client = { "", "", "", {}, false };

	const auto nameValidator = [this](const QString& name)
	{
		for (int i = 0; i < m_model.length(); i++)
		{
			if (m_model[i].name.compare(name, Qt::CaseInsensitive) == 0)
			{
				return false;
			}
		}

		return true;
	};

	const auto databaseNameValidator = [this](const QString& name)
	{
		for (int i = 0; i < m_model.length(); i++)
		{
			if (m_model[i].databaseName.compare(name, Qt::CaseInsensitive) == 0)
			{
				return false;
			}
		}

		return true;
	};

	ClientEditorDialog* editorWindow = new ClientEditorDialog(client, nameValidator, databaseNameValidator);
	connect(editorWindow, &ClientEditorDialog::clientChanged,
		[this](Core::Client client) { addClient(client); });

	editorWindow->show();
}

void ClientListEditorWidget::onClientsLoaded(Core::IBackendConnection::QueryId /*queryId*/, const QList<Core::Client>& clients)
{
	m_model = clients;

	updateData();

	hideProgressDialog();

	if (m_updating)
	{
		QMessageBox::information(this, "Сохранение данных", "Сохранение данных завершилось успешно.");
		m_updating = false;
	}
}

void ClientListEditorWidget::onClientsLoadFailed(Core::IBackendConnection::QueryId /*queryId*/, const QString& error)
{
	hideProgressDialog();

	QMessageBox::warning(this, "Загрузка данных", "Загрузка данных завершилась ошибкой: " + toLowerCase(error) + ".");
}

void ClientListEditorWidget::onClientsUpdated(Core::IBackendConnection::QueryId /*queryId*/)
{
	hideProgressDialog();

	loadData();
}

void ClientListEditorWidget::onClientsUpdateFailed(Core::IBackendConnection::QueryId /*queryId*/, const QString& error)
{
	hideProgressDialog();

	QMessageBox::warning(this, "Сохранение данных", "Сохранение данных завершилось ошибкой: " + toLowerCase(error) + ".");
}

void ClientListEditorWidget::onCleanupStatisticsSuccess(Core::IBackendConnection::QueryId queryId)
{
	if (queryId != m_cleanupStatisticsQueryId)
	{
		return;
	}

	hideProgressDialog();

	QMessageBox::information(this, "Очистка статистики", "Статистика успешно очищена.");
}

void ClientListEditorWidget::onCleanupStatisticsFailure(Core::IBackendConnection::QueryId queryId, const QString& error)
{
	if (queryId != m_cleanupStatisticsQueryId)
	{
		return;
	}

	hideProgressDialog();

	QMessageBox::warning(this, "Очистка статистики", "Очистка статистики завершилось ошибкой: " + toLowerCase(error) + ".");
}

void ClientListEditorWidget::updateClient(int index, const Core::Client& client)
{
	const Core::Client& sourceClient = m_model[index];
	if (client == sourceClient)
	{
		return;
	}

	showProgressDialog("Изменение данных", "Идет изменение данных. Пожалуйста, подождите.");

	const QMap<Core::Client, Core::Client> updated = {
		{ sourceClient, client }
	};
	m_backendConnection->updateClients({ updated, {}, {} });
}

void ClientListEditorWidget::addClient(const Core::Client& client)
{
	showProgressDialog("Добавление данных", "Идет добавление данных. Пожалуйста, подождите.");
	m_backendConnection->updateClients({ {}, {}, { client } });
}

void ClientListEditorWidget::onClientSelectionChanged()
{
	const QList<QListWidgetItem*> selectedItems = m_ui->listWidget->selectedItems();
	m_ui->cleanupStatisticsButton->setEnabled(selectedItems.size() == 1);

	m_ui->banButton->setEnabled(selectedItems.size() == 1);

	const bool isBanned = selectedItems.size() == 1 && m_model[m_ui->listWidget->currentRow()].banned;
	m_ui->banButton->setText(isBanned ? "Разблокировать" : "Заблокировать");
}

void ClientListEditorWidget::cleanupStatistics()
{
	const QList<QListWidgetItem*> selectedItems = m_ui->listWidget->selectedItems();
	Q_ASSERT(selectedItems.size() == 1);
	const QString clientName = selectedItems.first()->text();

	QMessageBox messageBox(QMessageBox::Question,
		"Очистить статистику",
		"Вы действительно хотите очистить статистику клиента \"" + clientName + "\"?",
		QMessageBox::Yes | QMessageBox::No,
		this);
	messageBox.setButtonText(QMessageBox::Yes, tr("Да"));
	messageBox.setButtonText(QMessageBox::No, tr("Нет"));

	const int answer = messageBox.exec();
	if (answer == QMessageBox::Yes)
	{
		const auto it = std::find_if(m_model.begin(), m_model.end(),
			[&clientName](const Core::Client& client){ return client.name == clientName; });
		Q_ASSERT(it != m_model.end());
		const Core::Client& client = *it;

		m_cleanupStatisticsQueryId = m_backendConnection->cleanupClientStatistics(client.databaseName);
	}
}

void ClientListEditorWidget::processBanSelected()
{
	const int selectedIndex = m_ui->listWidget->currentRow();
	Q_ASSERT(selectedIndex >= 0 && selectedIndex <= m_model.size());

	Core::Client client = m_model[selectedIndex];
	client.banned = !client.banned;
	updateClient(selectedIndex, client);
}
