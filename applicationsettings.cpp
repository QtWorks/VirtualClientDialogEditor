#include "applicationsettings.h"

const QString c_hostname = "appsettings/hostname";
const QString c_defaultHostname = "ws://vcappdemo.herokuapp.com/";

ApplicationSettings::ApplicationSettings(const QString& companyName, const QString& applicationName)
	: m_settings(companyName, applicationName)
{
}

QString ApplicationSettings::hostname() const
{
	return m_settings.value(c_hostname, c_defaultHostname).toString();
}

void ApplicationSettings::setHostname(const QString& hostname)
{
	m_settings.setValue(c_hostname, hostname);
}
