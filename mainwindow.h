#pragma once

#include "core/ibackendconnection.h"
#include "dialogeditor/dialoggraphicsinfostorage.h"
#include <QMainWindow>

namespace Ui
{
class MainWindow;
}

class ApplicationSettings;
class LoginDialog;
class SettingsDialog;
class ClientListEditorWidget;
class UserListEditorWidget;
class DialogsTabWidget;

class MainWindow
	: public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(ApplicationSettings* settings, IBackendConnectionSharedPtr backendConnection, DialogGraphicsInfoStoragePtr dialogGraphicsInfoStoragePtr, QWidget* parent = 0);
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
	ClientListEditorWidget* m_clientListEditorWidget;
	UserListEditorWidget* m_usersListEditorWidget;
	DialogsTabWidget* m_dialogsTabWidget;
};
