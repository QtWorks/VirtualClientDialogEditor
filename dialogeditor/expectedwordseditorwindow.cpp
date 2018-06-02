#include "expectedwordseditorwindow.h"
#include "ui_expectedwordseditorwindow.h"
#include "expectedwordeditor.h"
#include "logger.h"

ExpectedWordsEditorWindow::ExpectedWordsEditorWindow(const Core::ExpectedWordsNode& expectedWords, QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::ExpectedWordsEditorWindow)
	, m_expectedWords(expectedWords)
{
	m_ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose, true);

	connect(m_ui->useHintCheckBox, &QCheckBox::stateChanged, this, &ExpectedWordsEditorWindow::onUseHintCheckboxChecked);
	connect(m_ui->useHintCheckBox, &QCheckBox::stateChanged, this, &ExpectedWordsEditorWindow::validate);

	m_ui->useHintCheckBox->setChecked(expectedWords.customHint());
	onUseHintCheckboxChecked(expectedWords.customHint());

	if (expectedWords.customHint())
	{
		m_ui->hintTextEdit->setText(expectedWords.hint());
	}

	connect(m_ui->hintTextEdit, &QTextEdit::textChanged, this, &ExpectedWordsEditorWindow::onHintChanged);
	connect(m_ui->hintTextEdit, &QTextEdit::textChanged, this, &ExpectedWordsEditorWindow::validate);

	QFontMetrics fontMetrics = QFontMetrics(m_ui->hintTextEdit->font());
	m_ui->hintTextEdit->setMaximumHeight(fontMetrics.lineSpacing() * 4);
	m_ui->hintTextEdit->setMaximumHeight(fontMetrics.lineSpacing() * 4);

	m_ui->listWidget->setStyleSheet("QListWidget::item { border-bottom: 1px solid black; }");
	m_ui->listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

	for (const Core::ExpectedWords& expectedWord : m_expectedWords.expectedWords())
	{
		addItemWidget(expectedWord);
	}

	connect(m_ui->addButton, &QPushButton::clicked, this, ExpectedWordsEditorWindow::onAddItemClicked);

	QIcon warningIcon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
	QPixmap warningPixmap = warningIcon.pixmap(QSize(16, 16));
	m_ui->errorIconLabel->setPixmap(warningPixmap);

	m_ui->buttonBox->button(QDialogButtonBox::Save)->setText("Сохранить");
	m_ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отменить");
	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, ExpectedWordsEditorWindow::onSaveClicked);
	connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, ExpectedWordsEditorWindow::onCancelClicked);

	validate();
}

ExpectedWordsEditorWindow::~ExpectedWordsEditorWindow()
{
	delete m_ui;
}

void ExpectedWordsEditorWindow::onUseHintCheckboxChecked(bool checked)
{
	m_expectedWords.setCustomHint(checked);
	if (checked)
	{
		m_expectedWords.setHint(m_ui->hintTextEdit->toPlainText().trimmed());
	}

	m_ui->hintTextEdit->setDisabled(!checked);
	m_ui->hintLabel->setDisabled(!checked);
}

void ExpectedWordsEditorWindow::onHintChanged()
{
	m_expectedWords.setHint(m_ui->hintTextEdit->toPlainText().trimmed());
}

void ExpectedWordsEditorWindow::onAddItemClicked()
{	
	if (!m_itemEditors.isEmpty() && m_itemEditors.last()->expectedWords().words.trimmed().isEmpty())
	{
		return;
	}

	QList<Core::ExpectedWords> expectedWords = m_expectedWords.expectedWords();
	expectedWords.append(Core::ExpectedWords("", 0));
	m_expectedWords.setExpectedWords(expectedWords);

	addItemWidget(m_expectedWords.expectedWords().last());

	validate();

	m_ui->listWidget->scrollToBottom();

	m_itemEditors.last()->setFocus();
}

void ExpectedWordsEditorWindow::onSaveClicked()
{
	QList<Core::ExpectedWords> words;
	for (ExpectedWordEditor* editor : m_itemEditors)
	{
		words.append(editor->expectedWords());
	}
	m_expectedWords.setExpectedWords(words);

	const QString hint = m_ui->hintTextEdit->toPlainText().trimmed();
	m_expectedWords.setHint(hint.isEmpty() ? "" : hint);

	hide();

	emit accepted(m_expectedWords);
}

void ExpectedWordsEditorWindow::onCancelClicked()
{
	hide();
	emit rejected();
}

void ExpectedWordsEditorWindow::addItemWidget(const Core::ExpectedWords& item)
{
	ExpectedWordEditor* editorItem = new ExpectedWordEditor(item, m_ui->listWidget);
	m_itemEditors.append(editorItem);

	connect(editorItem, &ExpectedWordEditor::changed, [this]()
	{

		validate();
	});

	connect(editorItem, &ExpectedWordEditor::removed, [this, editorItem]()
	{
		int itemIndex = m_itemEditors.indexOf(editorItem);
		Q_ASSERT(m_ui->listWidget->takeItem(itemIndex));
		m_itemEditors.removeAt(itemIndex);

		validate();
	});

	QListWidgetItem* widget = new QListWidgetItem();
	widget->setSizeHint(QSize(editorItem->width(), editorItem->maximumHeight()));

	m_ui->listWidget->addItem(widget);
	m_ui->listWidget->setItemWidget(widget, editorItem);
}

void ExpectedWordsEditorWindow::validate()
{
	QList<Core::ExpectedWords> expectedWords;
	for (ExpectedWordEditor* editor : m_itemEditors)
	{
		expectedWords.append(editor->expectedWords());
	}

	QString error;

	Core::ExpectedWordsNode expectedWordsNode = m_expectedWords;
	expectedWordsNode.setExpectedWords(expectedWords);
	if (!expectedWordsNode.validate(error))
	{
		setError(error);
	}
	else
	{
		removeError();
	}
}

void ExpectedWordsEditorWindow::setError(const QString& error)
{
	m_ui->errorIconLabel->show();

	m_ui->errorTextLabel->show();
	m_ui->errorTextLabel->setText(error);

	m_ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
}

void ExpectedWordsEditorWindow::removeError()
{
	m_ui->errorIconLabel->hide();

	m_ui->errorTextLabel->hide();
	m_ui->errorTextLabel->setText("");

	m_ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
}
