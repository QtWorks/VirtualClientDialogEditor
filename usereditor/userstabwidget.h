#pragma once

#include "ui_userstabwidget.h"
#include "userlisteditorwidget.h"

class UsersTabWidget
	: public QWidget
{
	Q_OBJECT

public:
	UsersTabWidget(IBackendConnectionSharedPtr backendConnection, QWidget* parent = nullptr);

	void loadData();

private slots:
	void setClients(const QList<Core::Client>& clients);

	void updateClientsList(Core::IBackendConnection::QueryId queryId, const QList<Core::Client>& clients);
	void updateUsersList(int clientIndex);
	void updateUsersList(bool showAll);

	QString clientIdToName(QString clientId);

private:
	Ui::UsersTabWidget m_ui;
	UserListEditorWidget m_listEditorWidget;
	QString m_currentClient;
	QList<Core::Client> m_clients;
};
