#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include "core/ibackendconnection.h"
#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog
	: public QDialog
{
	Q_OBJECT

public:
	explicit LoginDialog(IBackendConnectionSharedPtr backendConnection, QWidget* parent = 0);
	~LoginDialog();

private:
	Ui::LoginDialog* m_ui;
	IBackendConnectionSharedPtr m_backendConnection;
};

#endif // LOGINWIDGET_H
