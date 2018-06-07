#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include "core/ibackendconnection.h"
#include <QDialog>

namespace Ui {
class LoginDialog;
}

class WaitingSpinnerWidget;

class ApplicationSettings;
class SettingsDialog;

class LoginDialog
	: public QDialog
{
	Q_OBJECT

public:
	LoginDialog(IBackendConnectionSharedPtr backendConnection, QWidget* parent = 0);
	~LoginDialog();

	void setSettings(ApplicationSettings* settings);

private:
	void showSpinner();
	void hideSpinner();

private slots:
	void showSettingsDialog();

	void onLoginClicked();
	void onLoggedIn(Core::IBackendConnection::QueryId queryId);
	void onLoginFailed(Core::IBackendConnection::QueryId queryId, const QString& error);

private:
	Ui::LoginDialog* m_ui;
	IBackendConnectionSharedPtr m_backendConnection;
	WaitingSpinnerWidget* m_waitingSpinner;
	Core::IBackendConnection::QueryId m_queryId;

	ApplicationSettings* m_settings;
	SettingsDialog* m_settingsDialog;
};

#endif // LOGINWIDGET_H
