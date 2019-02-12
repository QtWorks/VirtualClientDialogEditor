#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logindialog.h"
#include "settingsdialog.h"
#include "clienteditor/clientlisteditorwidget.h"
#include "clienteditor/groupstabwidget.h"
#include "dialogeditor/dialogstabwidget.h"
#include "usereditor/userstabwidget.h"

#include <QDesktopWidget>

MainWindow::MainWindow(ApplicationSettings* settings, IBackendConnectionSharedPtr backendConnection,
	DialogGraphicsInfoStoragePtr dialogGraphicsInfoStoragePtr, QWidget* parent)
	: QMainWindow(parent)
	, m_ui(new Ui::MainWindow)
	, m_loginDialog(new LoginDialog(backendConnection, this))
	, m_settingsDialog(new SettingsDialog(this))
	, m_clientListEditorWidget(new ClientListEditorWidget(backendConnection, this))
	, m_groupsTabWidget(new GroupsTabWidget(backendConnection, this))
	, m_usersTabWidget(new UsersTabWidget(backendConnection, this))
	, m_dialogsTabWidget(new DialogsTabWidget(backendConnection, dialogGraphicsInfoStoragePtr, this))
{
	m_ui->setupUi(this);

	m_ui->tabWidget->addTab(m_clientListEditorWidget, "Клиенты");
	m_ui->tabWidget->addTab(m_groupsTabWidget, "Группы");
	m_ui->tabWidget->addTab(m_usersTabWidget, "Пользователи");
	m_ui->tabWidget->addTab(m_dialogsTabWidget, "Диалоги");

	m_settingsAction = m_ui->menuBar->addAction("Настройки");
	connect(m_settingsAction, &QAction::triggered, this, &MainWindow::showSettingsWindow);

	QRect scr = QApplication::desktop()->screenGeometry();
	move(scr.center() - rect().center());

	connect(m_loginDialog, &QDialog::finished, this, &MainWindow::onLoginDialogFinished);
	m_loginDialog->setSettings(settings);

	m_settingsDialog->setSettings(settings);
	m_dialogsTabWidget->setSettings(settings);

	connect(backendConnection.get(), &Core::IBackendConnection::clientsLoaded,
		[this](Core::IBackendConnection::QueryId /*queryId*/, const QList<Core::Client>& /*clients*/)
		{
			m_usersTabWidget->loadData();

			m_dialogsTabWidget->loadData();
		});
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
		m_clientListEditorWidget->loadData();
		m_usersTabWidget->loadData();
		m_dialogsTabWidget->loadData();
	}
	else if (code == QDialog::Rejected)
	{
		close();
	}
}
