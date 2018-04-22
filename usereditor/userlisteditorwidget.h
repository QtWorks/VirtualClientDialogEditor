#pragma once

#include "core/ibackendconnection.h"
#include "core/user.h"
#include "listeditorwidget.h"
#include "listdatamodel.h"

class UserListEditorWidget
	: public ListEditorWidget
{
public:
	UserListEditorWidget(IBackendConnectionSharedPtr backendConnection, QWidget* parent = nullptr);

	void loadData();

private:
	virtual QStringList items() const override;
	virtual void saveChanges() override;
	virtual bool itemHasChanges(const QString& item) const override;
	virtual void revertChanges(const QString& item) override;
	virtual void revertAllChanges() override;

private slots:
	void onItemEditRequested(const QString& username);
	void onItemsRemoveRequested(const QStringList& users);
	void onItemCreateRequested();

	void onDiffRecordsCountChanged(int count);

	void onUsersLoaded(const QList<Core::User>& users);
	void onUsersLoadFailed(const QString& error);
	void onUsersUpdated();
	void onUsersUpdateFailed(const QString& error);

private:
	IBackendConnectionSharedPtr m_backendConnection;

	typedef ListDataModel<Core::User> UserListDataModel;
	UserListDataModel m_model;

	bool m_updating;
};
