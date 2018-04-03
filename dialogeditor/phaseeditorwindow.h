#ifndef PHASEEDITORWINDOW_H
#define PHASEEDITORWINDOW_H

#include "core/dialog.h"
#include <QDialog>

namespace Ui {
class PhaseEditorWindow;
}

class PhaseEditorWindow
	: public QDialog
{
	Q_OBJECT

public:
	PhaseEditorWindow(const Core::PhaseNode& phase, QWidget* parent = 0);
	~PhaseEditorWindow();

signals:
	void accepted(Core::PhaseNode phase);
	void rejected();

private slots:
	void onSaveClicked();
	void onCancelClicked();
	void onNameChanged();
	void onScoreChanged();

private:
	void setError(const QString& message);
	void removeError();

private:
	Ui::PhaseEditorWindow* m_ui;
	Core::PhaseNode m_phase;
};

#endif // PHASEEDITORWINDOW_H
