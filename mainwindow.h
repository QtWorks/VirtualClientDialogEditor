#pragma once

#include "core/ibackendconnection.h"
#include <QMainWindow>

namespace Ui
{
class MainWindow;
}

class LoginDialog;
class UserListEditorWidget;
class DialogListEditorWidget;

class MainWindow
    : public QMainWindow
{
    Q_OBJECT

public:
	explicit MainWindow(IBackendConnectionSharedPtr backendConnection, QWidget* parent = 0);
    ~MainWindow();

	void show();

private slots:
	void onLoginDialogFinished(int code);

private:
    Ui::MainWindow* m_ui;

	LoginDialog* m_loginDialog;
	UserListEditorWidget* m_usersListEditorWidget;
	DialogListEditorWidget* m_dialogsListEditorWidget;
};
