#include "groupstabwidget.h"

using namespace Core;

GroupsTabWidget::GroupsTabWidget(IBackendConnectionSharedPtr backendConnection, QWidget* parent)
	: QWidget(parent)
	, m_listEditorWidget(new GroupsListEditorWidget(backendConnection, parent))
{
	m_ui.setupUi(this);
	m_ui.verticalLayout->addWidget(m_listEditorWidget);

	connect(backendConnection.get(), &IBackendConnection::clientsLoaded, this, &GroupsTabWidget::updateClientsList);

	connect(m_ui.clientsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GroupsTabWidget::setCurrentClient);
}

void GroupsTabWidget::updateClientsList(IBackendConnection::QueryId /*queryId*/, const QList<Core::Client>& clients)
{
	m_clients = clients;

	int currentClientIndex = -1;
	if (clients.contains(m_currentClient))
	{
		currentClientIndex = m_ui.clientsComboBox->findData(m_currentClient.id);
	}

	m_ui.clientsComboBox->clear();

	for (const auto& client : clients)
	{
		m_ui.clientsComboBox->addItem(client.name, client.id);
	}

	if (currentClientIndex != -1)
	{
		m_ui.clientsComboBox->setCurrentIndex(currentClientIndex);
	}
}

void GroupsTabWidget::setCurrentClient(int clientIndex)
{
	if (clientIndex == -1)
	{
		return;
	}

	m_currentClient = m_clients[clientIndex];
	m_listEditorWidget->setCurrentClient(m_currentClient);
}
