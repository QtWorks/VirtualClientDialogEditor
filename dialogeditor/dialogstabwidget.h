#pragma once

#include "ui_dialogstabwidget.h"
#include "dialoglisteditorwidget.h"

class ApplicationSettings;

class DialogsTabWidget
	: public QWidget
{
	Q_OBJECT

public:
	DialogsTabWidget(IBackendConnectionSharedPtr backendConnection, DialogGraphicsInfoStoragePtr dialogGraphicsInfoStorage, QWidget* parent = nullptr);

	void loadData();
	void setSettings(ApplicationSettings* settings);

private slots:
	void updateClientsList(Core::IBackendConnection::QueryId queryId, const QList<Core::Client>& clients);
	void updateDialogsList(int clientIndex);

private:
	Ui::DialogsTabWidget m_ui;
	DialogListEditorWidget m_listEditorWidget;
	Core::Client m_currentClient;
	QList<Core::Client> m_clients;
};
