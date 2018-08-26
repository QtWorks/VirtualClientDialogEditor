#pragma once

#include "core/ibackendconnection.h"
#include "core/dialog.h"
#include "listeditorwidget.h"
#include "listdatamodel.h"

class DialogListEditorWidget
	: public ListEditorWidget
{
public:
	DialogListEditorWidget(IBackendConnectionSharedPtr backendConnection, QWidget* parent = nullptr);

	void loadData();

private:
	virtual QStringList items() const override;
	virtual void removeItems(const QStringList& items) override;

private slots:
	void onItemEditRequested(const QString& dialogName);
	void onItemCreateRequested();

	void onDialogsLoaded(Core::IBackendConnection::QueryId queryId, const QList<Core::Dialog>& dialogs);
	void onDialogsLoadFailed(Core::IBackendConnection::QueryId queryId, const QString& error);
	void onDialogsUpdated(Core::IBackendConnection::QueryId queryId);
	void onDialogsUpdateFailed(Core::IBackendConnection::QueryId queryId, const QString& error);

private:
	IBackendConnectionSharedPtr m_backendConnection;

	typedef ListDataModel<Core::Dialog> DialogListDataModel;
	DialogListDataModel m_model;

	bool m_updating;
};
