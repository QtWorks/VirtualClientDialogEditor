#include "logindialog.h"
#include "ui_logindialog.h"

#include <QMessageBox>

LoginDialog::LoginDialog(IBackendConnectionSharedPtr backendConnection, QWidget* parent)
	: QDialog(parent)
	, m_ui(new Ui::LoginDialog)
	, m_backendConnection(backendConnection)
{
	m_ui->setupUi(this);
	m_ui->rememberMeCheckBox->setVisible(false);

	setModal(true);

	connect(m_ui->loginButton, &QPushButton::clicked, [this]()
	{
		m_ui->loginButton->setDisabled(true);

		m_backendConnection->logIn(m_ui->usernameEdit->text(), m_ui->passwordEdit->text());
	});

	connect(m_backendConnection.get(), &Core::IBackendConnection::loggedIn, [this]()
	{
		accept();
	});

	connect(m_backendConnection.get(), &Core::IBackendConnection::error, [this](const QString& message)
	{
		QMessageBox::critical(this, "Ошибка авторизации", message);
		m_ui->loginButton->setDisabled(false);
	});
}

LoginDialog::~LoginDialog()
{
	delete m_ui;
}
