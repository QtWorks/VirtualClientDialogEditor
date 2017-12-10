#include "expectedwordeditor.h"
#include "ui_expectedwordeditor.h"

ExpectedWordEditor::ExpectedWordEditor(const QString& text, QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::ExpectedWordEditor)
{
	m_ui->setupUi(this);

	m_ui->textEdit->setText(text);

	QFontMetrics fontMetrics = QFontMetrics(m_ui->textEdit->font());
	setMinimumHeight(fontMetrics.lineSpacing() * 3.5);
	setMaximumHeight(fontMetrics.lineSpacing() * 5.5);

	connect(m_ui->textEdit, &QTextEdit::textChanged, [this]()
	{
		QString text = m_ui->textEdit->toPlainText().trimmed();
		emit changed(text);
	});

	connect(m_ui->removeButton, &QPushButton::clicked, [this]()
	{
		emit removed();
	});
}

ExpectedWordEditor::~ExpectedWordEditor()
{
	delete m_ui;
}

QString ExpectedWordEditor::text() const
{
	return m_ui->textEdit->toPlainText();
}

void ExpectedWordEditor::setFocus()
{
	m_ui->textEdit->setFocus();
}
