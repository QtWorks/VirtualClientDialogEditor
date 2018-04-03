#include "expectedwordeditor.h"
#include "ui_expectedwordeditor.h"

ExpectedWordEditor::ExpectedWordEditor(const Core::ExpectedWords& expectedWords, QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::ExpectedWordEditor)
	, m_expectedWords(expectedWords)
{
	m_ui->setupUi(this);

	m_ui->textEdit->setText(m_expectedWords.words);

	QFontMetrics fontMetrics = QFontMetrics(m_ui->textEdit->font());
	setMinimumHeight(fontMetrics.lineSpacing() * 3.5);
	setMaximumHeight(fontMetrics.lineSpacing() * 5.5);

	connect(m_ui->textEdit, &QTextEdit::textChanged, [this]()
	{
		m_expectedWords.words = m_ui->textEdit->toPlainText().trimmed();
		emit changed(m_expectedWords);
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
	return m_expectedWords.words;
}

void ExpectedWordEditor::setFocus()
{
	m_ui->textEdit->setFocus();
}
