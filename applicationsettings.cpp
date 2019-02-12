#include "applicationsettings.h"

const QString c_hostname = "appsettings/hostname";
const QString c_defaultHostname = "ws://vcappdemo.herokuapp.com/";

const QString c_phaseErrorReplica = "appsettings/phaseErrorReplica";
const QString c_phaseErrorPenalty = "appsettings/phaseErrorPenalty";
const QString c_phaseFinishingExpectedWords = "appsettings/phaseFinishingExpectedWords";
const QString c_phaseFinishingReplica = "appsettings/phaseFinishingReplica";
const QString c_phaseRepeatReplica = "appsettings/phaseRepeatReplica";

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

QString ApplicationSettings::phaseErrorReplica() const
{
	return m_settings.value(c_phaseErrorReplica, "").toString();
}

void ApplicationSettings::setPhaseErrorReplica(const QString& value)
{
	m_settings.setValue(c_phaseErrorReplica, value);
}

double ApplicationSettings::phaseErrorPenalty() const
{
	return m_settings.value(c_phaseErrorPenalty, 0.0).toDouble();
}

void ApplicationSettings::setPhaseErrorPenalty(double value)
{
	m_settings.setValue(c_phaseErrorPenalty, value);
}

QString ApplicationSettings::phaseFinishingExpectedWords() const
{
	return m_settings.value(c_phaseFinishingExpectedWords, "").toString();
}

void ApplicationSettings::setPhaseFinishingExpectedWords(const QString& value)
{
	m_settings.setValue(c_phaseFinishingExpectedWords, value);
}

QString ApplicationSettings::phaseFinishingReplica() const
{
	return m_settings.value(c_phaseFinishingReplica, "").toString();
}

void ApplicationSettings::setPhaseFinishingReplica(const QString& value)
{
	m_settings.setValue(c_phaseFinishingReplica, value);
}

QString ApplicationSettings::phaseRepeatReplica() const
{
	return m_settings.value(c_phaseRepeatReplica, "").toString();
}

void ApplicationSettings::setPhaseRepeatReplica(const QString& value)
{
	m_settings.setValue(c_phaseRepeatReplica, value);
}
