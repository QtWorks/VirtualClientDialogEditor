#include "clientreplicaeditor.h"
#include "ui_clientreplicaeditor.h"

#include <QPushButton>

ClientReplicaEditor::ClientReplicaEditor(const Core::ClientReplicaNode& replica, QWidget* parent)
	: QDialog(parent)
	, m_ui(new Ui::ClientReplicaEditor)
	, m_replica(replica)
{
	m_ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose, true);
	setModal(true);

	m_ui->plainTextEdit->document()->setPlainText(m_replica.replica());
	m_ui->plainTextEdit->setMinimumHeight(50);
	m_ui->plainTextEdit->setMinimumWidth(300);
	connect(m_ui->plainTextEdit, &QPlainTextEdit::textChanged, this, &ClientReplicaEditor::onReplicaChanged);

	QIcon warningIcon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
	QPixmap warningPixmap = warningIcon.pixmap(QSize(16, 16));
	m_ui->errorIconLabel->setPixmap(warningPixmap);

	m_ui->buttonBox->button(QDialogButtonBox::Save)->setText("Сохранить");
	m_ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отменить");
	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, ClientReplicaEditor::onSaveClicked);
	connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, ClientReplicaEditor::onCancelClicked);

	resize(400, m_ui->plainTextEdit->minimumHeight() + m_ui->buttonBox->height() + m_ui->verticalLayout->spacing());
	updateControls();
}

ClientReplicaEditor::~ClientReplicaEditor()
{
	delete m_ui;
}

void ClientReplicaEditor::onReplicaChanged()
{
	m_replica.setReplica(m_ui->plainTextEdit->toPlainText().trimmed());

	updateControls();
	emit changed();
}

void ClientReplicaEditor::onSaveClicked()
{
	QString error;
	Q_ASSERT(m_replica.validate(error));

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
	QString error;
	const bool saveAllowed = m_replica.validate(error);

	m_ui->errorIconLabel->setVisible(!saveAllowed);

	m_ui->errorTextLabel->setVisible(!saveAllowed);
	m_ui->errorTextLabel->setText(error);

	m_ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(saveAllowed);
}
