#include "dialogstabwidget.h"

using namespace Core;

DialogsTabWidget::DialogsTabWidget(IBackendConnectionSharedPtr backendConnection, DialogGraphicsInfoStoragePtr dialogGraphicsInfoStorage, QWidget* parent)
	: QWidget(parent)
	, m_listEditorWidget(backendConnection, dialogGraphicsInfoStorage, parent)
{
	m_ui.setupUi(this);
	m_ui.verticalLayout->addWidget(&m_listEditorWidget);

	connect(m_ui.clientsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DialogsTabWidget::updateDialogsList);
	connect(backendConnection.get(), &IBackendConnection::clientsLoaded, this, &DialogsTabWidget::updateClientsList);
	connect(backendConnection.get(), &IBackendConnection::dialogsLoaded, [this]() { m_listEditorWidget.setCurrentClient(m_currentClient); });
}

void DialogsTabWidget::loadData()
{
	m_listEditorWidget.loadData();
}

void DialogsTabWidget::updateClientsList(IBackendConnection::QueryId /*queryId*/, const QList<Core::Client>& clients)
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

void DialogsTabWidget::updateDialogsList(int clientIndex)
{
	m_currentClient = m_clients[clientIndex];
	m_listEditorWidget.setCurrentClient(m_currentClient);
}
