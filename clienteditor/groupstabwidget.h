#pragma once

#include "ui_groupstabwidget.h"
#include "groupslisteditorwidget.h"
#include "core/ibackendconnection.h"

class GroupsTabWidget
	: public QWidget
{
	Q_OBJECT

public:
	GroupsTabWidget(IBackendConnectionSharedPtr backendConnection, QWidget* parent = 0);

private slots:
	void updateClientsList(Core::IBackendConnection::QueryId queryId, const QList<Core::Client>& clients);
	void setCurrentClient(int clientIndex);

private:
	Ui::GroupsTabWidget m_ui;

	QList<Core::Client> m_clients;
	Core::Client m_currentClient;

	GroupsListEditorWidget* m_listEditorWidget;
};
