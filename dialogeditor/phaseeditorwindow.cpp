#include "phaseeditorwindow.h"
#include "ui_phaseeditorwindow.h"

#include <QPushButton>

PhaseEditorWindow::PhaseEditorWindow(const Core::PhaseNode& phase, QWidget* parent)
	: QDialog(parent)
	, m_ui(new Ui::PhaseEditorWindow)
	, m_phase(phase)
{
	m_ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose, true);

	QIcon warningIcon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
	QPixmap warningPixmap = warningIcon.pixmap(QSize(16, 16));
	m_ui->errorIconLabel->setPixmap(warningPixmap);
	m_ui->errorIconLabel->hide();

	m_ui->nameLineEdit->setText(m_phase.name());
	connect(m_ui->nameLineEdit, &QLineEdit::textChanged, this, PhaseEditorWindow::onNameChanged);

	m_ui->scoreLineEdit->setText(QString::number(m_phase.score()));
	m_ui->scoreLineEdit->setValidator(new QIntValidator(0, INT_MAX, this));
	connect(m_ui->scoreLineEdit, &QLineEdit::textChanged, this, PhaseEditorWindow::onScoreChanged);

	m_ui->buttonBox->button(QDialogButtonBox::Save)->setText("Сохранить");
	m_ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отменить");
	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, PhaseEditorWindow::onSaveClicked);
	connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, PhaseEditorWindow::onCancelClicked);
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
	emit accepted(m_phase);
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
