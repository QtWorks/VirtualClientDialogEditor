#pragma once

#include <QSettings>
#include <QString>

class ApplicationSettings
{
public:
	ApplicationSettings(const QString& companyName = "CompanyNameStub", const QString& applicationName = "VCDialogEditor");

	QString hostname() const;
	void setHostname(const QString& hostname);

	QString phaseErrorReplica() const;
	void setPhaseErrorReplica(const QString& value);

	double phaseErrorPenalty() const;
	void setPhaseErrorPenalty(double value);

	QString phaseFinishingExpectedWords() const;
	void setPhaseFinishingExpectedWords(const QString& value);

	QString phaseFinishingReplica() const;
	void setPhaseFinishingReplica(const QString& value);

	QString phaseRepeatReplica() const;
	void setPhaseRepeatReplica(const QString& value);

private:
	QSettings m_settings;
};
