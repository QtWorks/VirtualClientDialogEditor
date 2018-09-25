#ifndef DIALOGSTABWIDGET_H
#define DIALOGSTABWIDGET_H

#include "ui_dialogstabwidget.h"
#include "dialoglisteditorwidget.h"

class DialogsTabWidget
	: public QWidget
{
	Q_OBJECT

public:
	DialogsTabWidget(IBackendConnectionSharedPtr backendConnection, QWidget* parent = nullptr);

	void loadData();

private slots:
	void updateClientsList(Core::IBackendConnection::QueryId queryId, const QMap<QString, QList<Core::Dialog>>& dialogs);
	void updateDialogsList(const QString& client);

private:
	Ui::DialogsTabWidget m_ui;
	DialogListEditorWidget m_listEditorWidget;
	QString m_currentClient;
};

#endif // DIALOGSTABWIDGET_H
