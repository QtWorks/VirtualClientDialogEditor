#ifndef PHASEEDITORWINDOW_H
#define PHASEEDITORWINDOW_H

#include "core/dialog.h"
#include <QDialog>

class QCheckBox;
class QLineEdit;
class QPlainTextEdit;

namespace Ui {
class PhaseEditorWindow;
}

class PhaseEditorWindow
	: public QDialog
{
	Q_OBJECT

public:
	PhaseEditorWindow(const Core::PhaseNode& phase, const Core::Dialog& dialog, bool replicationEnabled, QWidget* parent = 0);
	~PhaseEditorWindow();

signals:
	void accepted(const Core::PhaseNode& phase);
	void accepted(const Core::PhaseNode& phase, const Core::ErrorReplica& globalErrorReplica, const Optional<QString>& globalRepeatReplica);

	void rejected();
	void changed();

private slots:
	void onSaveClicked();
	void onCancelClicked();
	void onNameChanged();
	void onScoreChanged();
	void onRepeatOnInsufficientScoreChanged();
	void onErrorReplicaChanged();
	void onErrorPenaltyChanged();
	void onFinishingExpectedWordsChanged();
	void onFinishingReplicaChanged();
	void onRepeatReplicaChanged();
	void validate();

private:
	void setError(const QString& message);
	void removeError();
	void bindControls();

	void initCheckbox(QCheckBox* checkbox, bool isOverridenField);

	typedef void (PhaseEditorWindow::*PointerToMemeberFunction)();
	void initLineEdit(QLineEdit* textEdit, const QString& value, bool isOverridenField, PointerToMemeberFunction onTextChanged);
	void initPlainTextEdit(QPlainTextEdit* textEdit, const QString& value, bool isOverridenField, PointerToMemeberFunction onTextChanged);

private:
	Ui::PhaseEditorWindow* m_ui;
	Core::PhaseNode m_phase;
	std::reference_wrapper<const Core::Dialog> m_dialog;
	bool m_replicationEnabled { false };
};

#endif // PHASEEDITORWINDOW_H
