#ifndef DIALOGSTABWIDGET_H
#define DIALOGSTABWIDGET_H

#include "ui_dialogstabwidget.h"
#include "dialoglisteditorwidget.h"

class DialogsTabWidget
	: public QWidget
{
	Q_OBJECT

public:
	DialogsTabWidget(IBackendConnectionSharedPtr backendConnection, DialogGraphicsInfoStoragePtr dialogGraphicsInfoStorage, QWidget* parent = nullptr);

	void loadData();

private slots:
	void updateClientsList(Core::IBackendConnection::QueryId queryId, const QList<Core::Client>& clients);
	void updateDialogsList(int clientIndex);

private:
	Ui::DialogsTabWidget m_ui;
	DialogListEditorWidget m_listEditorWidget;
	Core::Client m_currentClient;
	QList<Core::Client> m_clients;
};

#endif // DIALOGSTABWIDGET_H
