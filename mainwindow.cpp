#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logindialog.h"
#include "settingsdialog.h"
#include "dialogeditor/dialoglisteditorwidget.h"
#include "usereditor/userlisteditorwidget.h"

#include <QDesktopWidget>

MainWindow::MainWindow(ApplicationSettings* settings, IBackendConnectionSharedPtr backendConnection, QWidget* parent)
	: QMainWindow(parent)
	, m_ui(new Ui::MainWindow)
	, m_loginDialog(new LoginDialog(backendConnection, this))
	, m_settingsDialog(new SettingsDialog(this))
	, m_usersListEditorWidget(new UserListEditorWidget(backendConnection, this))
	, m_dialogsListEditorWidget(new DialogListEditorWidget(backendConnection, this))
{
	m_ui->setupUi(this);

	m_ui->tabWidget->addTab(m_dialogsListEditorWidget, "Диалоги");
	m_ui->tabWidget->addTab(m_usersListEditorWidget, "Пользователи");

	m_settingsAction = m_ui->menuBar->addAction("Настройки");
	connect(m_settingsAction, &QAction::triggered, this, &MainWindow::showSettingsWindow);

	QRect scr = QApplication::desktop()->screenGeometry();
	move(scr.center() - rect().center());

	connect(m_loginDialog, &QDialog::finished, this, &MainWindow::onLoginDialogFinished);
	m_loginDialog->setSettings(settings);

	m_settingsDialog->setSettings(settings);
}

MainWindow::~MainWindow()
{
	delete m_ui;
}

void MainWindow::show()
{
	QMainWindow::show();

	if (m_loginDialog)
	{
		m_loginDialog->show();
	}
}

void MainWindow::showSettingsWindow()
{
	m_settingsDialog->show();
}

void MainWindow::onLoginDialogFinished(int code)
{
	m_loginDialog = nullptr;

	if (code == QDialog::Accepted)
	{
		m_usersListEditorWidget->loadData();
		m_dialogsListEditorWidget->loadData();
	}
	else if (code == QDialog::Rejected)
	{
		close();
	}
}
