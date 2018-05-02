#include "logindialog.h"
#include "ui_logindialog.h"
#include "waitingspinnerwidget.h"

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
		showSpinner();

		m_backendConnection->logIn(m_ui->usernameEdit->text(), m_ui->passwordEdit->text());
	});

	connect(m_backendConnection.get(), &Core::IBackendConnection::loggedIn, [this]()
	{
		QTimer::singleShot(1000, [this]() {
			hideSpinner();
			accept();
		});
	});

	connect(m_backendConnection.get(), &Core::IBackendConnection::logInFailed, [this](const QString& message)
	{
		QTimer::singleShot(1000, [this, &message]() {
			hideSpinner();
			QMessageBox::critical(this, "Ошибка авторизации", message);
		});
	});
}

LoginDialog::~LoginDialog()
{
	delete m_ui;
}

void LoginDialog::showSpinner()
{
	m_waitingSpinner = new WaitingSpinnerWidget(Qt::NonModal, this, false, true);
	m_waitingSpinner->setRoundness(70.0);
	m_waitingSpinner->setMinimumTrailOpacity(15.0);
	m_waitingSpinner->setTrailFadePercentage(70.0);
	m_waitingSpinner->setNumberOfLines(12);
	m_waitingSpinner->setLineLength(10);
	m_waitingSpinner->setLineWidth(5);
	m_waitingSpinner->setInnerRadius(10);
	m_waitingSpinner->setRevolutionsPerSecond(1);
	m_waitingSpinner->setColor(QColor(81, 4, 71));
	m_waitingSpinner->start();
}

void LoginDialog::hideSpinner()
{
	m_waitingSpinner->stop();
	delete m_waitingSpinner;
}
