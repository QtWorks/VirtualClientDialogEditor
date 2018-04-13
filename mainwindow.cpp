#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "listeditorwidget.h"
#include "dialogeditor/dialogeditorwindow.h"
#include "usereditor/usereditordialog.h"

#include "core/dialogjsonwriter.h"

#include <QDesktopWidget>
#include <QMessageBox>

MainWindow::MainWindow(IBackendConnectionSharedPtr backendConnection, QWidget* parent)
	: QMainWindow(parent)
	, m_ui(new Ui::MainWindow)
	, m_backendConnection(backendConnection)
	, m_loginDialog(new LoginDialog(m_backendConnection, this))
	, m_usersListEditorWidget(new ListEditorWidget(this))
	, m_dialogsListEditorWidget(new ListEditorWidget(this))
{
	m_ui->setupUi(this);

	m_ui->tabWidget->addTab(m_dialogsListEditorWidget, "Диалоги");
	m_ui->tabWidget->addTab(m_usersListEditorWidget, "Пользователи");

	QRect scr = QApplication::desktop()->screenGeometry();
	move(scr.center() - rect().center());

	connect(m_loginDialog, &QDialog::finished, this, &MainWindow::onLoginDialogFinished);

	connect(m_backendConnection.get(), &Core::IBackendConnection::onUsersReaded, this, &MainWindow::onUsersReaded);
	connect(m_backendConnection.get(), &Core::IBackendConnection::onDialogsReaded, this, &MainWindow::onDialogsReaded);

	connect(m_usersListEditorWidget, &ListEditorWidget::itemEditRequested, this, &MainWindow::onUserEditRequested);
	connect(m_usersListEditorWidget, &ListEditorWidget::itemRemoveRequested, this, &MainWindow::onUserRemoveRequested);
	connect(m_usersListEditorWidget, &ListEditorWidget::itemCreateRequested, this, &MainWindow::onUserCreateRequested);

	connect(m_dialogsListEditorWidget, &ListEditorWidget::itemEditRequested, this, &MainWindow::onDialogEditRequested);
	connect(m_dialogsListEditorWidget, &ListEditorWidget::itemRemoveRequested, this, &MainWindow::onDialogRemoveRequested);
	connect(m_dialogsListEditorWidget, &ListEditorWidget::itemCreateRequested, this, &MainWindow::onDialogCreateRequested);
}

MainWindow::~MainWindow()
{
	delete m_ui;
}

void MainWindow::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	//if (m_loginDialog)
	//{
	//	m_loginDialog->show();
	//}
	onLoginDialogFinished(QDialog::Accepted);
}

void MainWindow::onLoginDialogFinished(int code)
{
	m_loginDialog = nullptr;

	if (code == QDialog::Accepted)
	{
		m_backendConnection->readUsers();
		m_backendConnection->readDialogs();
	}
	else if (code == QDialog::Rejected)
	{
		close();
	}
}

void MainWindow::onUsersReaded(const QList<Core::User>& users)
{
	m_users = users;

	QStringList userList;
	for (const Core::User& user : users)
	{
		userList.append(user.name());
	}

	m_usersListEditorWidget->setItems(userList);
}

void MainWindow::onDialogsReaded(const QList<Core::Dialog>& dialogs)
{
	m_dialogs = dialogs;

	QStringList dialogList;
	for (const Core::Dialog& dialog : dialogs)
	{
		dialogList.append(dialog.printableName());
	}

	m_dialogsListEditorWidget->setItems(dialogList);
}

void MainWindow::onUserEditRequested(QString username)
{
	const auto it = std::find_if(m_users.begin(), m_users.end(),
		[&username](const Core::User& user)
		{
			return user.name() == username;
		});
	Q_ASSERT(it != m_users.end());

	UserEditorDialog* dialog = new UserEditorDialog(*it, this);

	connect(dialog, &UserEditorDialog::userChanged, [this, it, username](Core::User user)
	{
		*it = user;
		m_usersListEditorWidget->updateItem(username, user.name());
	});

	dialog->show();
}

void MainWindow::onUserRemoveRequested(QString username)
{
	const auto it = std::find_if(m_users.begin(), m_users.end(),
		[&username](const Core::User& user)
		{
			return user.name() == username;
		});
	Q_ASSERT(it != m_users.end());

	m_users.removeAt(std::distance(m_users.begin(), it));
	m_usersListEditorWidget->removeItem(username);
}

void MainWindow::onUserCreateRequested()
{
	UserEditorDialog* dialog = new UserEditorDialog({ }, this);

	connect(dialog, &UserEditorDialog::userChanged, [this](Core::User user)
	{
		// TODO: same name
		m_users.append(user);
		m_usersListEditorWidget->addItem(user.name());
	});

	dialog->show();
}

void MainWindow::onDialogEditRequested(QString dialogName)
{
	const auto it = std::find_if(m_dialogs.begin(), m_dialogs.end(),
		[&dialogName](const Core::Dialog& dialog)
		{
			return dialog.printableName() == dialogName;
		});
	Q_ASSERT(it != m_dialogs.end());

	// TODO: add to map dialogName -> editorWindow
	DialogEditorWindow* window = new DialogEditorWindow(*it);

	connect(window, &DialogEditorWindow::dialogChanged,
		[this, it, dialogName](Core::Dialog dialog)
		{
			*it = dialog;

			Core::DialogJsonWriter writer;
			LOG << writer.write(dialog);

			m_dialogsListEditorWidget->updateItem(dialogName, dialog.printableName());
		});

	window->show();
}

void MainWindow::onDialogRemoveRequested(QString dialogName)
{
	const auto it = std::find_if(m_dialogs.begin(), m_dialogs.end(),
		[&dialogName](const Core::Dialog& dialog)
		{
			return dialog.printableName() == dialogName;
		});
	Q_ASSERT(it != m_dialogs.end());

	m_dialogs.removeAt(std::distance(m_dialogs.begin(), it));
	m_dialogsListEditorWidget->removeItem(dialogName);
}

void MainWindow::onDialogCreateRequested()
{
	DialogEditorWindow* window = new DialogEditorWindow({ "", Core::Dialog::Difficulty::Easy, { } });

	connect(window, &DialogEditorWindow::dialogChanged,
		[this](Core::Dialog dialog)
		{
			// TODO: dialog is invalid if there is another dialog with the same combination of name and difficulty
			m_dialogs.append(dialog);

			Core::DialogJsonWriter writer;
			LOG << writer.write(dialog);

			m_dialogsListEditorWidget->addItem(dialog.printableName());
		});

	window->show();
}
