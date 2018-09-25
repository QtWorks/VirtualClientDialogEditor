#ifndef CLIENTEDITORDIALOG_H
#define CLIENTEDITORDIALOG_H

#include "core/client.h"
#include "ui_clienteditordialog.h"
#include <QDialog>
#include <functional>

class ClientEditorDialog
	: public QDialog
{
	Q_OBJECT

public:
	typedef std::function<bool(const QString&)> UniquenessValidator;
	ClientEditorDialog(const Core::Client& client,
		const UniquenessValidator& nameValidator,
		const UniquenessValidator& databaseNameValidator,
		QWidget* parent = 0);
	~ClientEditorDialog();

signals:
	void clientChanged(Core::Client client);
	void canceled();

private slots:
	void saveChanges();
	void discardChanges();

private:
	Ui::ClientEditorDialog* m_ui;
	Core::Client m_client;
	UniquenessValidator m_nameValidator;
	UniquenessValidator m_databaseNameValidator;
};

#endif // CLIENTEDITORDIALOG_H
