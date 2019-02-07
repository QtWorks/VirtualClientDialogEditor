#include "clientlisteditorwidget.h"
#include "clienteditordialog.h"
#include "core/ibackendconnection.h"

#include "logger.h"

#include <QMessageBox>

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
	const Core::Client client = { "", "", "", {} };

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
