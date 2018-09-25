#pragma once

#include "core/ibackendconnection.h"
#include "core/dialog.h"
#include "listeditorwidget.h"

class DialogListEditorWidget
	: public ListEditorWidget
{
public:
	DialogListEditorWidget(IBackendConnectionSharedPtr backendConnection, QWidget* parent = nullptr);

	void loadData();
	void setCurrentClient(const QString& client);

private:
	virtual QStringList items() const override;
	virtual void removeItems(const QStringList& items) override;

private slots:
	void onItemEditRequested(const QString& dialogName);
	void onItemCreateRequested();

	void onDialogsLoaded(Core::IBackendConnection::QueryId queryId, const QMap<QString, QList<Core::Dialog>>& dialogs);
	void onDialogsLoadFailed(Core::IBackendConnection::QueryId queryId, const QString& error);
	void onDialogsUpdated(Core::IBackendConnection::QueryId queryId);
	void onDialogsUpdateFailed(Core::IBackendConnection::QueryId queryId, const QString& error);

private:
	void updateDialog(int index, const Core::Dialog& dialog);
	void addDialog(const Core::Dialog& dialog);

private:
	IBackendConnectionSharedPtr m_backendConnection;

	typedef QMap<QString, QList<Core::Dialog>> DialogListDataModel;
	DialogListDataModel m_model;
	QString m_currentClient;

	bool m_updating;
};
