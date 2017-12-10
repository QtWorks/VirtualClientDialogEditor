#include "clientreplicaeditor.h"
#include "ui_clientreplicaeditor.h"

#include <QPushButton>

ClientReplicaEditor::ClientReplicaEditor(const QString& replica, QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::ClientReplicaEditor)
	, m_replica(replica)
{
	m_ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);

	m_ui->textEdit->setText(m_replica);
	m_ui->textEdit->setMinimumHeight(50);
	m_ui->textEdit->setMinimumWidth(300);
	connect(m_ui->textEdit, &QTextEdit::textChanged, [this]()
	{
		m_replica = m_ui->textEdit->toPlainText().trimmed();

		updateControls();
	});

	QIcon warningIcon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
	QPixmap warningPixmap = warningIcon.pixmap(QSize(16, 16));
	m_ui->errorIconLabel->setPixmap(warningPixmap);

	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, ClientReplicaEditor::onSaveClicked);
	connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, ClientReplicaEditor::onCancelClicked);

	resize(400, m_ui->textEdit->minimumHeight() + m_ui->buttonBox->height() + m_ui->verticalLayout->spacing());
	updateControls();
}

ClientReplicaEditor::~ClientReplicaEditor()
{
	delete m_ui;
}

void ClientReplicaEditor::onSaveClicked()
{
	Q_ASSERT(!m_replica.trimmed().isEmpty());

	hide();
	emit accepted(m_replica);
}

void ClientReplicaEditor::onCancelClicked()
{
	hide();
	emit rejected();
}

void ClientReplicaEditor::updateControls()
{
	const bool saveAllowed = !m_replica.isEmpty();
	m_ui->errorIconLabel->setVisible(!saveAllowed);
	m_ui->errorTextLabel->setVisible(!saveAllowed);
	m_ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(saveAllowed);
}
