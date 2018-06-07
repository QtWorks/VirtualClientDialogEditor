#pragma once

#include <QSettings>
#include <QString>

class ApplicationSettings
{
public:
	ApplicationSettings(const QString& companyName = "CompanyNameStub", const QString& applicationName = "VCDialogEditor");

	QString hostname() const;
	void setHostname(const QString& hostname);

private:
	QSettings m_settings;
};
