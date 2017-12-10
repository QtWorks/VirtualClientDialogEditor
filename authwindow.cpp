#include "authwindow.h"
#include "ui_authwindow.h"

AuthWindow::AuthWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_ui(new Ui::AuthWindow)
{
	m_ui->setupUi(this);

	connect(m_ui->loginButton, &QPushButton::clicked, this, &AuthWindow::onLoginClicked);
}

AuthWindow::~AuthWindow()
{
	delete m_ui;
}

void AuthWindow::onLoginClicked()
{
	close(); // here or by signal?
	emit onAuthenticated();
}
