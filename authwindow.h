#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H

#include <QMainWindow>

namespace Ui
{
class AuthWindow;
}

class AuthWindow
	: public QMainWindow
{
	Q_OBJECT

public:
	explicit AuthWindow(QWidget* parent = 0);
	~AuthWindow();

signals:
	void onAuthenticated();

private slots:
	void onLoginClicked();

private:
	Ui::AuthWindow* m_ui;
};

#endif // AUTHWINDOW_H
