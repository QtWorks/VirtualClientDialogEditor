#include "phaseeditorwindow.h"
#include "ui_phaseeditorwindow.h"
#include "core/dialog.h"

#include <QPushButton>

PhaseEditorWindow::PhaseEditorWindow(const Core::PhaseNode& phase, const Core::Dialog& dialog, QWidget* parent)
	: QDialog(parent)
	, m_ui(new Ui::PhaseEditorWindow)
	, m_phase(phase)
	, m_dialog(dialog)
{
	m_ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose, true);

	QIcon warningIcon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
	QPixmap warningPixmap = warningIcon.pixmap(QSize(16, 16));
	m_ui->errorIconLabel->setPixmap(warningPixmap);
	m_ui->errorIconLabel->hide();

	connect(m_ui->nameLineEdit, &QLineEdit::textChanged, this, &PhaseEditorWindow::onNameChanged);

	m_ui->scoreLineEdit->setValidator(new QIntValidator(0, INT_MAX, this));
	connect(m_ui->scoreLineEdit, &QLineEdit::textChanged, this, &PhaseEditorWindow::onScoreChanged);

	connect(m_ui->errorReplicaTextEdit, &QTextEdit::textChanged, this, &PhaseEditorWindow::onErrorReplicaChanged);

	m_ui->buttonBox->button(QDialogButtonBox::Save)->setText("Сохранить");
	m_ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отменить");
	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, PhaseEditorWindow::onSaveClicked);
	connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, PhaseEditorWindow::onCancelClicked);

	connect(this, &PhaseEditorWindow::changed, this, &PhaseEditorWindow::validate);

	updateInterface();
}

PhaseEditorWindow::~PhaseEditorWindow()
{
	delete m_ui;
}

void PhaseEditorWindow::onSaveClicked()
{
	QString error;
	Q_ASSERT(m_phase.validate(error));

	hide();
	emit accepted(m_phase, m_ui->errorReplicaCheckBox->isChecked());
}

void PhaseEditorWindow::onCancelClicked()
{
	hide();
	emit rejected();
}

void PhaseEditorWindow::onNameChanged()
{
	m_phase.setName(m_ui->nameLineEdit->text().trimmed());
	emit changed();
}

void PhaseEditorWindow::onScoreChanged()
{
	const QString scoreString = m_ui->scoreLineEdit->text().trimmed();
	if (scoreString.isEmpty())
	{
		m_ui->scoreLineEdit->setText("0");
		return;
	}

	bool ok = false;
	const double score = scoreString.toDouble(&ok);
	Q_ASSERT(ok);

	m_phase.setScore(score);
	emit changed();
}

void PhaseEditorWindow::onErrorReplicaChanged()
{
	m_phase.setErrorReplica(m_ui->errorReplicaTextEdit->toPlainText().trimmed());
	emit changed();
}

void PhaseEditorWindow::validate()
{
	QString error;
	if (!m_phase.validate(error))
	{
		setError(error);
	}
	else
	{
		removeError();
	}
}

void PhaseEditorWindow::setError(const QString& message)
{
	m_ui->errorIconLabel->show();

	m_ui->errorTextLabel->show();
	m_ui->errorTextLabel->setText(message);

	m_ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
}

void PhaseEditorWindow::removeError()
{
	m_ui->errorIconLabel->hide();

	m_ui->errorTextLabel->hide();

	m_ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
}

void PhaseEditorWindow::updateInterface()
{
	m_ui->errorIconLabel->hide();

	m_ui->nameLineEdit->setText(m_phase.name());
	m_ui->nameLineEdit->setFocus();

	m_ui->scoreLineEdit->setText(QString::number(m_phase.score()));

	const Core::Dialog& dialog = m_dialog.get();

	const bool hasCustomErrorReplicas = std::any_of(dialog.phases.begin(), dialog.phases.end(),
		[](const Core::PhaseNode& phase) { return phase.hasErrorReplica(); });
	m_ui->errorReplicaCheckBox->setChecked(!hasCustomErrorReplicas);

	const QString errorReplica = m_phase.hasErrorReplica() ? m_phase.errorReplica() : dialog.errorReplica;
	m_ui->errorReplicaTextEdit->setText(errorReplica);
}
