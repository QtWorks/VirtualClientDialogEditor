#pragma once

#include "core/ibackendconnection.h"
#include <QMainWindow>

namespace Ui
{
class MainWindow;
}

class ApplicationSettings;
class LoginDialog;
class SettingsDialog;
class UserListEditorWidget;
class DialogListEditorWidget;

class MainWindow
	: public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(ApplicationSettings* settings, IBackendConnectionSharedPtr backendConnection, QWidget* parent = 0);
	~MainWindow();

	void show();

private slots:
	void showSettingsWindow();
	void onLoginDialogFinished(int code);

private:
	Ui::MainWindow* m_ui;
	QAction* m_settingsAction;

	LoginDialog* m_loginDialog;
	SettingsDialog* m_settingsDialog;
	UserListEditorWidget* m_usersListEditorWidget;
	DialogListEditorWidget* m_dialogsListEditorWidget;
};
