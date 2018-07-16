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
	PhaseEditorWindow(const Core::PhaseNode& phase, const Core::Dialog& dialog, QWidget* parent = 0);
	~PhaseEditorWindow();

signals:
	void accepted(const Core::PhaseNode& phase, QVector<Core::ErrorReplica::Field> replicatingFields);
	void rejected();
	void changed();

private slots:
	void onSaveClicked();
	void onCancelClicked();
	void onNameChanged();
	void onScoreChanged();
	void onErrorReplicaChanged();
	void onFinishingExpectedWordsChanged();
	void onFinishingReplicaChanged();
	void onContinuationExpectedWordsChanged();
	void validate();

private:
	void setError(const QString& message);
	void removeError();
	void updateInterface();

private:
	Ui::PhaseEditorWindow* m_ui;
	Core::PhaseNode m_phase;
	std::reference_wrapper<const Core::Dialog> m_dialog;
};

#endif // PHASEEDITORWINDOW_H
