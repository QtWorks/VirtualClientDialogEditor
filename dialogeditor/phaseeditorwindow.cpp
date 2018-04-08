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

	m_ui->nameLineEdit->setText(m_phase.name);
	connect(m_ui->nameLineEdit, &QLineEdit::textChanged, this, PhaseEditorWindow::onNameChanged);

	m_ui->scoreLineEdit->setText(QString::number(m_phase.score));
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
	Q_ASSERT(!m_phase.name.trimmed().isEmpty());

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
	const QString name = m_ui->nameLineEdit->text().trimmed();
	if (name.isEmpty())
	{
		setError("Имя не может быть пустым");
		return;
	}

	removeError();
	m_phase.name = name;
}

void PhaseEditorWindow::onScoreChanged()
{
	const QString scoreString = m_ui->scoreLineEdit->text().trimmed();
	if (scoreString.isEmpty())
	{
		setError("Кол-во баллов не может быть пустым");
		return;
	}

	bool ok = false;
	const double score = scoreString.toDouble(&ok);
	if (!ok)
	{
		setError("Кол-во баллов должно быть числом");
		return;
	}

	// TODO: add check for maximum available score

	removeError();
	m_phase.score = score;
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
