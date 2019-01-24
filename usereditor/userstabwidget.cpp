#include "userstabwidget.h"

using namespace Core;

QList<QString> clientIds(const QList<Client>& clients)
{
	QList<Client> sorted = clients;
	std::sort(sorted.begin(), sorted.end(), [](const Client& left, const Client& right)
	{
		return left.name < right.name;
	});

	QList<QString> result;

	for (const auto& client : sorted)
	{
		result.append(client.id);
	}

	result.prepend("");

	return result;
}

UsersTabWidget::UsersTabWidget(IBackendConnectionSharedPtr backendConnection, QWidget* parent)
	: QWidget(parent)
	, m_listEditorWidget(backendConnection, parent)
{
	m_ui.setupUi(this);
	m_ui.verticalLayout->addWidget(&m_listEditorWidget);

	connect(m_ui.clientsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, QOverload<int>::of(&UsersTabWidget::updateUsersList));

	connect(m_ui.showAllCheckBox, &QCheckBox::clicked, this,
		QOverload<bool>::of(&UsersTabWidget::updateUsersList));

	connect(m_ui.showAllCheckBox, &QCheckBox::clicked,
		m_ui.clientsComboBox, &QComboBox::setDisabled);

	connect(backendConnection.get(), &IBackendConnection::clientsLoaded, this, &UsersTabWidget::updateClientsList);
}

void UsersTabWidget::loadData()
{
	m_listEditorWidget.loadData();
}

void UsersTabWidget::setClients(const QList<Core::Client>& clients)
{
	m_clients = clients;
	m_listEditorWidget.setClients(clients);
}

void UsersTabWidget::updateClientsList(IBackendConnection::QueryId /*queryId*/, const QList<Client>& clients)
{
	setClients(clients);

	QList<QString> ids = clientIds(clients);

	int currentClientIndex = -1;
	if (ids.contains(m_currentClient))
	{
		currentClientIndex = m_ui.clientsComboBox->findText(m_currentClient);
	}

	m_ui.clientsComboBox->clear();
	for (const auto& clientId : ids)
	{
		m_ui.clientsComboBox->addItem(clientIdToName(clientId), clientId);
	}

	if (currentClientIndex != -1)
	{
		m_ui.clientsComboBox->setCurrentIndex(currentClientIndex);
	}
}

void UsersTabWidget::updateUsersList(int clientIndex)
{
	QString clientId = m_ui.clientsComboBox->itemData(clientIndex).toString();
	m_currentClient = clientId;
	m_listEditorWidget.setClientFilter(m_currentClient);
}

void UsersTabWidget::updateUsersList(bool showAll)
{
	if (showAll)
	{
		m_listEditorWidget.resetClientFilter();
	}
	else
	{
		m_listEditorWidget.setClientFilter(m_currentClient);
	}
}

QString UsersTabWidget::clientIdToName(QString clientId)
{
	if (clientId.isEmpty())
	{
		return "< Администраторы >";
	}

	auto it = std::find_if(m_clients.begin(), m_clients.end(), [&](const Core::Client& client) { return client.id == clientId; });
	return it == m_clients.end() ? "" : it->name;
}
