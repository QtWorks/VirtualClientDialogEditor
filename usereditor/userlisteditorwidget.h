#pragma once

#include "core/ibackendconnection.h"
#include "core/user.h"
#include "listeditorwidget.h"

class UserListEditorWidget
	: public ListEditorWidget
{
public:
	UserListEditorWidget(IBackendConnectionSharedPtr backendConnection, QWidget* parent = nullptr);

	void loadData();

	QList<Core::User> currentUsers() const;
	QList<Core::User> allUsers() const;
	void addUsers(const QList<Core::User>& currentUsers);

	void setClients(const QList<Core::Client>& clients);

	void setClientFilter(const QString& client);
	void resetClientFilter();

private:
	virtual QStringList items() const override;
	virtual void removeItems(const QStringList& items) override;

private slots:
	void onItemEditRequested(const QString& username);
	void onItemCreateRequested();

	void onUsersLoaded(Core::IBackendConnection::QueryId queryId, const QList<Core::User>& currentUsers);
	void onUsersLoadFailed(Core::IBackendConnection::QueryId queryId, const QString& error);
	void onUsersUpdated(Core::IBackendConnection::QueryId queryId);
	void onUsersUpdateFailed(Core::IBackendConnection::QueryId queryId, const QString& error);
	void onCleanupStatisticsSuccess(Core::IBackendConnection::QueryId queryId);
	void onCleanupStatisticsFailure(Core::IBackendConnection::QueryId queryId, const QString& error);

	void onUserSelectionChanged();
	void cleanupStatistics();
	void processBanSelected();

private:
	void updateUser(int index, const Core::User& user);
	void addUser(const Core::User& user);

private:
	IBackendConnectionSharedPtr m_backendConnection;

	typedef QList<Core::User> UserListDataModel;
	UserListDataModel m_originalModel;
	QString m_currentClient;
	UserListDataModel m_model;
	QList<Core::Client> m_clients;

	bool m_updating;

	Core::IBackendConnection::QueryId m_cleanupStatisticsQueryId { -1 };
};
