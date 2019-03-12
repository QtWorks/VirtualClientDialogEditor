#pragma once

#include "core/ibackendconnection.h"
#include "core/client.h"
#include "listeditorwidget.h"

class ClientListEditorWidget
	: public ListEditorWidget
{
public:
	ClientListEditorWidget(IBackendConnectionSharedPtr backendConnection, QWidget* parent = 0);

	void loadData();

private:
	virtual QStringList items() const override;
	virtual void removeItems(const QStringList& items) override;

private slots:
	void onItemEditRequested(const QString& clientName);
	void onItemCreateRequested();

	void onClientsLoaded(Core::IBackendConnection::QueryId queryId, const QList<Core::Client>& clients);
	void onClientsLoadFailed(Core::IBackendConnection::QueryId queryId, const QString& error);
	void onClientsUpdated(Core::IBackendConnection::QueryId queryId);
	void onClientsUpdateFailed(Core::IBackendConnection::QueryId queryId, const QString& error);
	void onCleanupStatisticsSuccess(Core::IBackendConnection::QueryId queryId);
	void onCleanupStatisticsFailure(Core::IBackendConnection::QueryId queryId, const QString& error);

	void onClientSelectionChanged();
	void cleanupStatistics();
	void processBanSelected();

private:
	void updateClient(int index, const Core::Client& client);
	void addClient(const Core::Client& client);

private:
	IBackendConnectionSharedPtr m_backendConnection;

	typedef QList<Core::Client> ClientListDataModel;
	ClientListDataModel m_model;

	bool m_updating;

	Core::IBackendConnection::QueryId m_cleanupStatisticsQueryId { -1 };
};
