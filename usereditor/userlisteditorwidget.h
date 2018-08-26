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

private:
	virtual QStringList items() const override;
	virtual void removeItems(const QStringList& items) override;

private slots:
	void onItemEditRequested(const QString& username);
	void onItemCreateRequested();

	void onUsersLoaded(Core::IBackendConnection::QueryId queryId, const QList<Core::User>& users);
	void onUsersLoadFailed(Core::IBackendConnection::QueryId queryId, const QString& error);
	void onUsersUpdated(Core::IBackendConnection::QueryId queryId);
	void onUsersUpdateFailed(Core::IBackendConnection::QueryId queryId, const QString& error);

private:
	void updateUser(int index, const Core::User& user);
	void addUser(const Core::User& user);

private:
	IBackendConnectionSharedPtr m_backendConnection;

	typedef QList<Core::User> UserListDataModel;
	UserListDataModel m_model;

	bool m_updating;
};
