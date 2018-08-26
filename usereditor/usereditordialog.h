#ifndef USEREDITORWIDGET_H
#define USEREDITORWIDGET_H

#include "core/user.h"
#include <QDialog>
#include <functional>

namespace Ui {
class UserEditorDialog;
}

class UserEditorDialog
	: public QDialog
{
	Q_OBJECT

public:
	typedef std::function<bool(const QString&)> UniquenessValidator;
	UserEditorDialog(const Core::User& user, const UniquenessValidator& validator, QWidget* parent = 0);
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
	UniquenessValidator m_uniquenessValidator;
};

#endif // USEREDITORWIDGET_H
