#include "phaseeditorwindow.h"
#include "ui_phaseeditorwindow.h"
#include <QPushButton>

PhaseEditorWindow::PhaseEditorWindow(const Phase& phase, QWidget* parent)
	: QDialog(parent)
	, m_ui(new Ui::PhaseEditorWindow)
	, m_phase(phase)
{
	m_ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);

	m_ui->lineEdit->setText(m_phase.name);

	connect(m_ui->lineEdit, &QLineEdit::textChanged, [this]()
	{
		m_phase.name = m_ui->lineEdit->text().trimmed();

		updateControls();
	});

	QIcon warningIcon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
	QPixmap warningPixmap = warningIcon.pixmap(QSize(16, 16));
	m_ui->errorIconLabel->setPixmap(warningPixmap);

	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, PhaseEditorWindow::onSaveClicked);
	connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, PhaseEditorWindow::onCancelClicked);

	resize(400, m_ui->lineEdit->minimumHeight() + m_ui->buttonBox->height() + m_ui->verticalLayout->spacing());
	updateControls();
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

void PhaseEditorWindow::updateControls()
{
	const bool saveAllowed = !m_phase.name.isEmpty();

	m_ui->errorIconLabel->setVisible(!saveAllowed);
	m_ui->errorTextLabel->setVisible(!saveAllowed);
	m_ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(saveAllowed);
}

