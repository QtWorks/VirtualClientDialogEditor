#pragma once

#include "ui_settingsdialog.h"
#include "applicationsettings.h"

class SettingsDialog
	: public QDialog
{
	Q_OBJECT

public:
	explicit SettingsDialog(QWidget* parent = 0);

	void setSettings(ApplicationSettings* settings);

private:
	void updateSettingsInterface();
	void updateSettings();
	void updateWarning();

private:
	Ui::SettingsDialog m_ui;
	ApplicationSettings* m_settings;
};
