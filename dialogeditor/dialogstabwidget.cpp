#include "dialogstabwidget.h"

using namespace Core;

DialogsTabWidget::DialogsTabWidget(IBackendConnectionSharedPtr backendConnection, DialogGraphicsInfoStoragePtr dialogGraphicsInfoStorage, QWidget* parent)
	: QWidget(parent)
	, m_listEditorWidget(backendConnection, dialogGraphicsInfoStorage, parent)
{
	m_ui.setupUi(this);
	m_ui.verticalLayout->addWidget(&m_listEditorWidget);

	connect(m_ui.clientsComboBox, &QComboBox::currentTextChanged, this, &DialogsTabWidget::updateDialogsList);

	connect(backendConnection.get(), &IBackendConnection::dialogsLoaded, this, &DialogsTabWidget::updateClientsList);
}

void DialogsTabWidget::loadData()
{
	m_listEditorWidget.loadData();
}

void DialogsTabWidget::updateClientsList(IBackendConnection::QueryId /*queryId*/, const QMap<QString, QList<Dialog>>& dialogs)
{
	QList<QString> clients = dialogs.keys();

	int currentClientIndex = -1;
	if (clients.contains(m_currentClient))
	{
		currentClientIndex = m_ui.clientsComboBox->findText(m_currentClient);
	}

	m_ui.clientsComboBox->clear();
	m_ui.clientsComboBox->addItems(clients);

	if (currentClientIndex != -1)
	{
		m_ui.clientsComboBox->setCurrentIndex(currentClientIndex);
	}
}

void DialogsTabWidget::updateDialogsList(const QString& client)
{
	m_currentClient = client;
	m_listEditorWidget.setCurrentClient(client);
}
