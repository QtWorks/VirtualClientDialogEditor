#include "phaseeditorwindow.h"
#include "ui_phaseeditorwindow.h"
#include "core/dialog.h"

#include <QPushButton>

namespace
{

const QString c_delimiter = ";";

QList<QString> splitExpectedWords(const QString& expectedWords)
{
	QList<QString> result = expectedWords.trimmed().split(c_delimiter, QString::SkipEmptyParts);
	for (QString& words : result)
	{
		words = words.trimmed();
	}
	return result;
}

}

PhaseEditorWindow::PhaseEditorWindow(const Core::PhaseNode& phase, const Core::Dialog& dialog, bool replicationEnabled, QWidget* parent)
	: QDialog(parent)
	, m_ui(new Ui::PhaseEditorWindow)
	, m_phase(phase)
	, m_dialog(dialog)
	, m_replicationEnabled(replicationEnabled)
{
	m_ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose, true);
	setModal(true);

	QIcon warningIcon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
	QPixmap warningPixmap = warningIcon.pixmap(QSize(16, 16));
	m_ui->errorIconLabel->setPixmap(warningPixmap);
	m_ui->errorIconLabel->hide();

	connect(m_ui->nameLineEdit, &QLineEdit::textChanged, this, &PhaseEditorWindow::onNameChanged);

	m_ui->scoreLineEdit->setValidator(new QIntValidator(0, INT_MAX, this));
	connect(m_ui->scoreLineEdit, &QLineEdit::textChanged, this, &PhaseEditorWindow::onScoreChanged);

	connect(m_ui->repeatOnInsufficientScoreCheckBox, &QCheckBox::clicked, this, &PhaseEditorWindow::onRepeatOnInsufficientScoreChanged);

	m_ui->buttonBox->button(QDialogButtonBox::Save)->setText("Сохранить");
	m_ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отменить");
	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, PhaseEditorWindow::onSaveClicked);
	connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, PhaseEditorWindow::onCancelClicked);

	connect(this, &PhaseEditorWindow::changed, this, &PhaseEditorWindow::validate);

	bindControls();
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

	if (!m_replicationEnabled)
	{
		emit accepted(m_phase);
		return;
	}

	Core::ErrorReplica globalErrorReplica;
	if (m_ui->errorReplicaCheckBox->isChecked())
	{
		globalErrorReplica.errorReplica = m_phase.errorReplica().errorReplica;
	}

	if (m_ui->errorPenaltyCheckBox->isChecked())
	{
		globalErrorReplica.errorPenalty = m_phase.errorReplica().errorPenalty;
	}

	if (m_ui->finishingExpectedWordsCheckBox->isChecked())
	{
		globalErrorReplica.finishingExpectedWords =*m_phase.errorReplica().finishingExpectedWords;
	}

	if (m_ui->finishingReplicaCheckBox->isChecked())
	{
		globalErrorReplica.finishingReplica = m_phase.errorReplica().finishingReplica;
	}

	Optional<QString> globalRepeatReplica;
	if (m_ui->repeatReplicaCheckBox->isChecked())
	{
		globalRepeatReplica = m_phase.repeatReplica();
	}

	emit accepted(m_phase, globalErrorReplica, globalRepeatReplica);
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

void PhaseEditorWindow::onRepeatOnInsufficientScoreChanged()
{
	m_phase.setRepeatOnInsufficientScore(m_ui->repeatOnInsufficientScoreCheckBox->isChecked());
	emit changed();
}

void PhaseEditorWindow::onErrorReplicaChanged()
{
	const QString replica = m_ui->errorReplicaPlainTextEdit->toPlainText().trimmed();
	m_phase.errorReplica().errorReplica = replica;
	emit changed();
}

void PhaseEditorWindow::onErrorPenaltyChanged()
{
	const QString penaltyString = m_ui->errorPenaltyLineEdit->text().trimmed();
	if (penaltyString.isEmpty())
	{
		m_ui->errorPenaltyLineEdit->setText("0");
		return;
	}

	bool ok = false;
	const double penalty = penaltyString.toDouble(&ok);
	Q_ASSERT(ok);

	m_phase.errorReplica().errorPenalty = penalty;
	emit changed();
}

void PhaseEditorWindow::onFinishingExpectedWordsChanged()
{
	const QList<QString> expectedWords = splitExpectedWords(m_ui->finishingExpectedWordsPlainTextEdit->toPlainText());
	m_phase.errorReplica().finishingExpectedWords = expectedWords;
	emit changed();
}

void PhaseEditorWindow::onFinishingReplicaChanged()
{
	const QString replica = m_ui->finishingReplicaPlainTextEdit->toPlainText().trimmed();
	m_phase.errorReplica().finishingReplica = replica;
	emit changed();
}

void PhaseEditorWindow::onRepeatReplicaChanged()
{
	const QString replica = m_ui->repeatReplicaPlainTextEdit->toPlainText().trimmed();
	m_phase.repeatReplica() = replica;
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

void PhaseEditorWindow::bindControls()
{
	m_ui->errorIconLabel->hide();

	m_ui->nameLineEdit->setText(m_phase.name());
	m_ui->nameLineEdit->setFocus();

	m_ui->scoreLineEdit->setText(QString::number(m_phase.score()));
	m_ui->repeatOnInsufficientScoreCheckBox->setChecked(m_phase.repeatOnInsufficientScore());

	const Core::Dialog& dialog = m_dialog.get();

	const Core::ErrorReplica& phaseError = m_phase.errorReplica();
	const Core::ErrorReplica& dialogError = dialog.errorReplica;

	initCheckbox(m_ui->errorReplicaCheckBox, phaseError.errorReplica);
	const QString errorReplica = phaseError.errorReplica ? *phaseError.errorReplica : *dialogError.errorReplica;
	initPlainTextEdit(m_ui->errorReplicaPlainTextEdit, errorReplica, phaseError.errorReplica, &PhaseEditorWindow::onErrorReplicaChanged);

	initCheckbox(m_ui->errorPenaltyCheckBox, phaseError.errorPenalty);
	m_ui->errorPenaltyLineEdit->setValidator(new QDoubleValidator(0, INT_MAX, 2, this));
	const QString errorPenalty = QString::number(phaseError.errorPenalty ? *phaseError.errorPenalty : *dialogError.errorPenalty);
	initLineEdit(m_ui->errorPenaltyLineEdit, errorPenalty, phaseError.errorPenalty, &PhaseEditorWindow::onErrorPenaltyChanged);

	initCheckbox(m_ui->finishingExpectedWordsCheckBox, phaseError.finishingExpectedWords);
	const QString finishingExpectedWords = (phaseError.finishingExpectedWords ? *phaseError.finishingExpectedWords : *dialogError.finishingExpectedWords).join(c_delimiter + " ");
	initPlainTextEdit(m_ui->finishingExpectedWordsPlainTextEdit, finishingExpectedWords, phaseError.finishingExpectedWords, &PhaseEditorWindow::onFinishingExpectedWordsChanged);

	initCheckbox(m_ui->finishingReplicaCheckBox, phaseError.finishingReplica);
	const QString finishingReplica = phaseError.finishingReplica ? *phaseError.finishingReplica : *dialogError.finishingReplica;
	initPlainTextEdit(m_ui->finishingReplicaPlainTextEdit, finishingReplica, phaseError.finishingReplica, &PhaseEditorWindow::onFinishingReplicaChanged);

	initCheckbox(m_ui->repeatReplicaCheckBox, m_phase.repeatReplica());
	const QString repeatReplica = m_phase.repeatReplica() ? *m_phase.repeatReplica() : *dialog.phaseRepeatReplica;
	initPlainTextEdit(m_ui->repeatReplicaPlainTextEdit, repeatReplica, m_phase.repeatReplica(), &PhaseEditorWindow::onRepeatReplicaChanged);
}

void PhaseEditorWindow::initCheckbox(QCheckBox* checkbox, bool isOverridenField)
{
	if (m_replicationEnabled)
	{
		checkbox->setVisible(true);
		checkbox->setChecked(!isOverridenField);
	}
	else
	{
		checkbox->setVisible(false);
	}
}

void PhaseEditorWindow::initPlainTextEdit(QPlainTextEdit* textEdit, const QString& value, bool isOverridenField, PointerToMemeberFunction onTextChanged)
{
	if (m_replicationEnabled || isOverridenField)
	{
		connect(textEdit, &QPlainTextEdit::textChanged, this, onTextChanged);
	}
	else
	{
		textEdit->setDisabled(true);
	}

	textEdit->document()->setPlainText(value);
}

void PhaseEditorWindow::initLineEdit(QLineEdit* textEdit, const QString& value, bool isOverridenField, PointerToMemeberFunction onTextChanged)
{
	if (m_replicationEnabled || isOverridenField)
	{
		connect(textEdit, &QLineEdit::textChanged, this, onTextChanged);
	}
	else
	{
		textEdit->setDisabled(true);
	}

	textEdit->setText(value);
}
