#ifndef SAVEASDIALOG_H
#define SAVEASDIALOG_H

#include "ui_saveasdialog.h"

#include "core/client.h"
#include "core/dialog.h"

class SaveAsDialog
	: public QDialog
{
	Q_OBJECT

public:
	typedef std::function<bool(const Core::Client&, const QString&)> NameValidator;

	SaveAsDialog(const QList<Core::Client>& clients, const Core::Client& selectedClient, NameValidator validator, QWidget* parent = 0);

signals:
	void accepted(Core::Client client, QString name);

private slots:
	void onSaveClicked();
	void onRejectClicked();

private:
	void showError(const QString& error);
	void hideError();

private:
	Ui::SaveAsDialog m_ui;

	QList<Core::Client> m_clients;
	NameValidator m_validator;
};

#endif // SAVEASDIALOG_H
