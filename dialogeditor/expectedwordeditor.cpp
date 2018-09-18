#include "expectedwordeditor.h"
#include "ui_expectedwordeditor.h"

ExpectedWordEditor::ExpectedWordEditor(const Core::ExpectedWords& expectedWords, QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::ExpectedWordEditor)
	, m_expectedWords(expectedWords)
{
	m_ui->setupUi(this);

	m_ui->wordsLineEdit->setText(m_expectedWords.words);

	QFontMetrics fontMetrics = QFontMetrics(m_ui->wordsLineEdit->font());
	setMinimumHeight(fontMetrics.lineSpacing() * 3.5);
	setMaximumHeight(fontMetrics.lineSpacing() * 5.5);

	connect(m_ui->wordsLineEdit, &QLineEdit::textChanged, this, &ExpectedWordEditor::onWordsChanged);

	m_ui->scoreLineEdit->setText(QString::number(m_expectedWords.score));
	m_ui->scoreLineEdit->setValidator(new QIntValidator(this));
	connect(m_ui->scoreLineEdit, &QLineEdit::textChanged, this, &ExpectedWordEditor::onScoreChanged);

	connect(m_ui->removeButton, &QPushButton::clicked, this, &ExpectedWordEditor::onRemoveClicked);
}

ExpectedWordEditor::~ExpectedWordEditor()
{
	delete m_ui;
}

Core::ExpectedWords ExpectedWordEditor::expectedWords() const
{
	return m_expectedWords;
}

void ExpectedWordEditor::setFocus()
{
	m_ui->wordsLineEdit->setFocus();
}

void ExpectedWordEditor::onWordsChanged()
{
	m_expectedWords.words = m_ui->wordsLineEdit->text().trimmed();
	emit changed();
}

void ExpectedWordEditor::onScoreChanged()
{
	const QString scoreString = m_ui->scoreLineEdit->text().trimmed();
	if (scoreString.isEmpty())
	{
		m_ui->scoreLineEdit->setText("0");
		return;
	}

	if (scoreString == "-")
	{
		return;
	}

	bool ok = false;
	const double score = scoreString.toDouble(&ok);
	if (!ok)
	{
		m_ui->scoreLineEdit->setText("0");
		return;
	}

	m_expectedWords.score = score;
	emit changed();
}

void ExpectedWordEditor::onRemoveClicked()
{
	emit removed();
}
