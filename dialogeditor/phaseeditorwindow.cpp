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

	connect(m_ui->errorReplicaPlainTextEdit, &QPlainTextEdit::textChanged, this, &PhaseEditorWindow::onErrorReplicaChanged);

	m_ui->errorPenaltyLineEdit->setValidator(new QDoubleValidator(0, INT_MAX, 2, this));
	connect(m_ui->errorPenaltyLineEdit, &QLineEdit::textChanged, this, &PhaseEditorWindow::onErrorPenaltyChanged);
	connect(m_ui->finishingExpectedWordsPlainTextEdit, &QPlainTextEdit::textChanged, this, &PhaseEditorWindow::onFinishingExpectedWordsChanged);
	connect(m_ui->finishingReplicaPlainTextEdit, &QPlainTextEdit::textChanged, this, &PhaseEditorWindow::onFinishingReplicaChanged);
	connect(m_ui->repeatReplicaPlainTextEdit, &QPlainTextEdit::textChanged, this, &PhaseEditorWindow::onRepeatReplicaChanged);

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

	QVector<Core::ErrorReplica::Field> replicatingFields;
	if (m_ui->errorReplicaCheckBox->isChecked())
	{
		replicatingFields << Core::ErrorReplica::Field::ErrorReplica;
	}

	if (m_ui->errorPenaltyCheckBox->isChecked())
	{
		replicatingFields << Core::ErrorReplica::Field::ErrorPenalty;
	}

	if (m_ui->finishingExpectedWordsCheckBox->isChecked())
	{
		replicatingFields << Core::ErrorReplica::Field::FinishingExpectedWords;
	}

	if (m_ui->finishingReplicaCheckBox->isChecked())
	{
		replicatingFields << Core::ErrorReplica::Field::FinishingReplica;
	}

	emit accepted(m_phase, replicatingFields, m_ui->repeatReplicaCheckBox->isChecked());
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
	const QString replica = m_ui->errorReplicaPlainTextEdit->toPlainText().trimmed();
	m_phase.errorReplica().setErrorReplica(replica);
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

	m_phase.errorReplica().setErrorPenalty(penalty);
	emit changed();
}

void PhaseEditorWindow::onFinishingExpectedWordsChanged()
{
	const QList<QString> expectedWords = splitExpectedWords(m_ui->finishingExpectedWordsPlainTextEdit->toPlainText());
	m_phase.errorReplica().setFinishingExpectedWords(expectedWords);
	emit changed();
}

void PhaseEditorWindow::onFinishingReplicaChanged()
{
	const QString replica = m_ui->finishingReplicaPlainTextEdit->toPlainText().trimmed();
	m_phase.errorReplica().setFinishingReplica(replica);
	emit changed();
}

void PhaseEditorWindow::onRepeatReplicaChanged()
{
	const QString replica = m_ui->repeatReplicaPlainTextEdit->toPlainText().trimmed();
	m_phase.setRepeatReplica(replica);
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

	const Core::ErrorReplica& phaseError = m_phase.errorReplica();
	const Core::ErrorReplica& dialogError = dialog.errorReplica;

	const auto hasCustomField = [this](Core::ErrorReplica::Field field)
	{
		return m_phase.errorReplica().has(field);
	};

	m_ui->errorReplicaCheckBox->setChecked(!hasCustomField(Core::ErrorReplica::Field::ErrorReplica));
	const QString errorReplica = phaseError.hasErrorReplica() ? phaseError.errorReplica() : dialogError.errorReplica();
	m_ui->errorReplicaPlainTextEdit->document()->setPlainText(errorReplica);

	m_ui->errorPenaltyCheckBox->setChecked(!hasCustomField(Core::ErrorReplica::Field::ErrorPenalty));
	const double errorPenalty = phaseError.hasErrorPenalty() ? phaseError.errorPenalty() : dialogError.errorPenalty();
	m_ui->errorPenaltyLineEdit->setText(QString::number(errorPenalty));

	m_ui->finishingExpectedWordsCheckBox->setChecked(!hasCustomField(Core::ErrorReplica::Field::FinishingExpectedWords));
	const QList<QString> finishingExpectedWords = phaseError.hasFinishingExpectedWords() ? phaseError.finishingExpectedWords() : dialogError.finishingExpectedWords();
	m_ui->finishingExpectedWordsPlainTextEdit->document()->setPlainText(finishingExpectedWords.join(c_delimiter + " "));

	m_ui->finishingReplicaCheckBox->setChecked(!hasCustomField(Core::ErrorReplica::Field::FinishingReplica));
	const QString finishingReplica = phaseError.hasFinishingReplica() ? phaseError.finishingReplica() : dialogError.finishingReplica();
	m_ui->finishingReplicaPlainTextEdit->document()->setPlainText(finishingReplica);

	m_ui->repeatReplicaCheckBox->setChecked(!m_phase.hasRepeatReplica());
	const QString repeatReplica = m_phase.hasRepeatReplica() ? m_phase.repeatReplica() : dialog.phaseRepeatReplica.toString();
	m_ui->repeatReplicaPlainTextEdit->document()->setPlainText(repeatReplica);
}
