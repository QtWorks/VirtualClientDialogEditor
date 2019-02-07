#pragma once

#include "core/ibackendconnection.h"
#include "core/client.h"
#include "listeditorwidget.h"

class GroupsListEditorWidget
	: public ListEditorWidget
{
public:
	GroupsListEditorWidget(IBackendConnectionSharedPtr backendConnection, QWidget* parent = 0);

	void loadData();
	void setCurrentClient(const Core::Client& client);

private:
	virtual QStringList items() const override;
	virtual void removeItems(const QStringList& items) override;

private slots:
	void onItemEditRequested(const QString& groupName);
	void onItemCreateRequested();

	void onClientsLoaded(Core::IBackendConnection::QueryId queryId, const QList<Core::Client>& clients);
	void onClientsLoadFailed(Core::IBackendConnection::QueryId queryId, const QString& error);
	void onClientsUpdated(Core::IBackendConnection::QueryId queryId);
	void onClientsUpdateFailed(Core::IBackendConnection::QueryId queryId, const QString& error);

private:

private:
	void updateGroup(int index, const Core::Group& group);
	void addGroup(const Core::Group& group);

private:
	IBackendConnectionSharedPtr m_backendConnection;

	QList<Core::Client> m_clients;
	Core::Client m_currentClient;

	bool m_updating;
};
