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

	const QString phaseErrorReplica = m_settings->phaseErrorReplica();
	m_ui.phaseErrorReplicaLineEdit->setText(phaseErrorReplica);

	const double phaseErrorPenalty = m_settings->phaseErrorPenalty();
	m_ui.phaseErrorPenaltyLineEdit->setText(QString::number(phaseErrorPenalty));

	const QString phaseFinishingExpectedWords = m_settings->phaseFinishingExpectedWords();
	m_ui.phaseFinishingExpectedWordsLineEdit->setText(phaseFinishingExpectedWords);

	const QString phaseFinishingReplica = m_settings->phaseFinishingReplica();
	m_ui.phaseFinishingReplicaLineEdit->setText(phaseFinishingReplica);

	const QString phaseRepeatReplica = m_settings->phaseRepeatReplica();
	m_ui.phaseRepeatReplicaLineEdit->setText(phaseRepeatReplica);
}

void SettingsDialog::updateSettings()
{
	const QString hostname = m_ui.hostnameLineEdit->text().trimmed();
	m_settings->setHostname(hostname);

	const QString phaseErrorReplica = m_ui.phaseErrorReplicaLineEdit->text().trimmed();
	m_settings->setPhaseErrorReplica(phaseErrorReplica);

	const double phaseErrorPenalty = m_ui.phaseErrorPenaltyLineEdit->text().trimmed().toDouble();
	m_settings->setPhaseErrorPenalty(phaseErrorPenalty);

	const QString phaseFinishingExpectedWords = m_ui.phaseFinishingExpectedWordsLineEdit->text().trimmed();
	m_settings->setPhaseFinishingExpectedWords(phaseFinishingExpectedWords);

	const QString phaseFinishingReplica = m_ui.phaseFinishingReplicaLineEdit->text().trimmed();
	m_settings->setPhaseFinishingReplica(phaseFinishingReplica);

	const QString phaseRepeatReplica = m_ui.phaseRepeatReplicaLineEdit->text().trimmed();
	m_settings->setPhaseRepeatReplica(phaseRepeatReplica);
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
