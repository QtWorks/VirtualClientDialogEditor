#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logindialog.h"
#include "dialogeditor/dialoglisteditorwidget.h"
#include "usereditor/userlisteditorwidget.h"

#include <QDesktopWidget>

MainWindow::MainWindow(IBackendConnectionSharedPtr backendConnection, QWidget* parent)
	: QMainWindow(parent)
	, m_ui(new Ui::MainWindow)
	, m_loginDialog(new LoginDialog(backendConnection, this))
	, m_usersListEditorWidget(new UserListEditorWidget(backendConnection, this))
	, m_dialogsListEditorWidget(new DialogListEditorWidget(backendConnection, this))
{
	m_ui->setupUi(this);

	m_ui->tabWidget->addTab(m_dialogsListEditorWidget, "Диалоги");
	m_ui->tabWidget->addTab(m_usersListEditorWidget, "Пользователи");

	QRect scr = QApplication::desktop()->screenGeometry();
	move(scr.center() - rect().center());

	connect(m_loginDialog, &QDialog::finished, this, &MainWindow::onLoginDialogFinished);
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
