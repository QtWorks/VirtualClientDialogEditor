#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core/ibackendconnection.h"
#include "logindialog.h"
#include "listeditorwidget.h"
#include <QMainWindow>

namespace Ui
{
class MainWindow;
}

class MainWindow
    : public QMainWindow
{
    Q_OBJECT

public:
	explicit MainWindow(IBackendConnectionSharedPtr backendConnection, QWidget* parent = 0);
    ~MainWindow();

private:
	virtual void showEvent(QShowEvent* event);

private slots:
	void onLoginDialogFinished(int code);

	void onUsersReaded(const QList<User>& users);
	void onDialogsReaded(const QList<Dialog>& dialogs);

	void onUserEditRequested(QString username);
	void onUserRemoveRequested(QString username);
	void onUserCreateRequested();

	void onDialogEditRequested(QString dialogName);
	void onDialogRemoveRequested(QString dialogName);
	void onDialogCreateRequested();

private:
    Ui::MainWindow* m_ui;

	IBackendConnectionSharedPtr m_backendConnection;
	LoginDialog* m_loginDialog;

	QList<User> m_users;
	ListEditorWidget* m_usersListEditorWidget;

	QList<Dialog> m_dialogs;
	ListEditorWidget* m_dialogsListEditorWidget;
};

#endif // MAINWINDOW_H