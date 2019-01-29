#include "saveasdialog.h"

#include <QDialogButtonBox>
#include <QPushButton>

SaveAsDialog::SaveAsDialog(const QList<Core::Client>& clients, const Core::Client& selectedClient, NameValidator validator, QWidget* parent)
	: QDialog(parent)
	, m_clients(clients)
	, m_validator(validator)
{
	m_ui.setupUi(this);

	for (const auto& client : clients)
	{
		m_ui.clientsComboBox->addItem(client.name, client.id);
	}
	m_ui.clientsComboBox->setCurrentIndex(clients.indexOf(selectedClient));

	QIcon warningIcon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
	QPixmap warningPixmap = warningIcon.pixmap(QSize(16, 16));
	m_ui.errorIconLabel->setPixmap(warningPixmap);
	hideError();

	QPushButton* saveButton = m_ui.buttonBox->button(QDialogButtonBox::Save);
	saveButton->setText("Сохранить");
	connect(saveButton, &QPushButton::clicked, this, &SaveAsDialog::onSaveClicked);

	QPushButton* rejectButton = m_ui.buttonBox->button(QDialogButtonBox::Cancel);
	rejectButton->setText("Отменить");
	connect(rejectButton, &QPushButton::clicked, this, &SaveAsDialog::onRejectClicked);
}

void SaveAsDialog::onSaveClicked()
{
	QString name = m_ui.dialogNameLineEdit->text().trimmed();
	if (name.isEmpty())
	{
		showError("Имя диалога не может быть пустым");
		return;
	}

	const Core::Client& currentClient = m_clients[m_ui.clientsComboBox->currentIndex()];
	if (!m_validator(currentClient, name))
	{
		showError("Имя диалога должно быть уникальным");
		return;
	}

	emit accepted(currentClient, name);
}

void SaveAsDialog::onRejectClicked()
{
	emit reject();
}

void SaveAsDialog::showError(const QString& error)
{
	m_ui.errorIconLabel->show();

	m_ui.errorTextLabel->setText(error);
	m_ui.errorTextLabel->show();
}

void SaveAsDialog::hideError()
{
	m_ui.errorIconLabel->hide();

	m_ui.errorTextLabel->setText("");
	m_ui.errorTextLabel->hide();
}
