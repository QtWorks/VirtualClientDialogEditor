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
	explicit UserEditorDialog(const Core::User& user, QWidget* parent = 0);
	~UserEditorDialog();

signals:
	void userChanged(Core::User user);
	void canceled();

private slots:
	void saveChanges();
	void discardChanges();

private:
	Ui::UserEditorDialog* m_ui;
	Core::User m_user;
};

#endif // USEREDITORWIDGET_H
