#include "settingsdialog.h"
#include <QPushButton>

SettingsDialog::SettingsDialog(QWidget* parent)
	: QDialog(parent)
{
	m_ui.setupUi(this);

	connect(m_ui.hostnameLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::updateWarning);

	m_ui.buttonBox->button(QDialogButtonBox::Save)->setText("Сохранить");
	m_ui.buttonBox->button(QDialogButtonBox::Cancel)->setText("Отменить");
	connect(m_ui.buttonBox, &QDialogButtonBox::accepted, this, [this]() { updateSettings(); emit accepted(); });
	connect(m_ui.buttonBox, &QDialogButtonBox::rejected, this, &QDialog::rejected);

	QIcon warningIcon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
	QPixmap warningPixmap = warningIcon.pixmap(QSize(16, 16));
	m_ui.warningIconLabel->setPixmap(warningPixmap);

	m_ui.warningIconLabel->hide();
	m_ui.warningTextLabel->hide();
}

void SettingsDialog::setSettings(ApplicationSettings* settings)
{
	m_settings = settings;
	updateSettingsInterface();
}

void SettingsDialog::updateSettingsInterface()
{
	const QString hostname = m_settings->hostname();
	m_ui.hostnameLineEdit->setText(hostname);
}

void SettingsDialog::updateSettings()
{
	const QString hostname = m_ui.hostnameLineEdit->text().trimmed();
	m_settings->setHostname(hostname);
}

void SettingsDialog::updateWarning()
{
	const bool settingsChanged = m_ui.hostnameLineEdit->text().trimmed() != m_settings->hostname();

	if (settingsChanged)
	{
		m_ui.warningIconLabel->show();
		m_ui.warningTextLabel->show();
	}
	else
	{
		m_ui.warningIconLabel->hide();
		m_ui.warningTextLabel->hide();
	}
}
