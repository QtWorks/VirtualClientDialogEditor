#ifndef USEREDITORWIDGET_H
#define USEREDITORWIDGET_H

#include "core/user.h"
#include <QDialog>

namespace Ui {
class UserEditorDialog;
}

class UserEditorDialog
	: public QDialog
{
	Q_OBJECT

public:
	explicit UserEditorDialog(const User& user, QWidget* parent = 0);
	~UserEditorDialog();

signals:
	void userChanged(User user);
	void canceled();

private:
	Ui::UserEditorDialog* m_ui;
	User m_user;
};

#endif // USEREDITORWIDGET_H
