#ifndef PHASEEDITORWINDOW_H
#define PHASEEDITORWINDOW_H

#include "core/phase.h"
#include <QDialog>

namespace Ui {
class PhaseEditorWindow;
}

class PhaseEditorWindow
	: public QDialog
{
	Q_OBJECT

public:
	PhaseEditorWindow(const Phase& phase, QWidget* parent = 0);
	~PhaseEditorWindow();

signals:
	void accepted(Phase phase);
	void rejected();

private slots:
	void onSaveClicked();
	void onCancelClicked();

private:
	void updateControls();

private:
	Ui::PhaseEditorWindow* m_ui;
	Phase m_phase;
};

#endif // PHASEEDITORWINDOW_H
