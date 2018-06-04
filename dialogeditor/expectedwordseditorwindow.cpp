#include "expectedwordseditorwindow.h"
#include "ui_expectedwordseditorwindow.h"
#include "expectedwordeditor.h"
#include "logger.h"

ExpectedWordsEditorWindow::ExpectedWordsEditorWindow(const Core::ExpectedWordsNode& expectedWords, QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::ExpectedWordsEditorWindow)
{
	m_ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose, true);

	connect(m_ui->useHintCheckBox, &QCheckBox::stateChanged, this, &ExpectedWordsEditorWindow::validate);
	connect(m_ui->useHintCheckBox, &QCheckBox::stateChanged, m_ui->hintLabel, &QLabel::setEnabled);
	connect(m_ui->useHintCheckBox, &QCheckBox::stateChanged, m_ui->hintTextEdit, &QTextEdit::setEnabled);

	connect(m_ui->hintTextEdit, &QTextEdit::textChanged, this, &ExpectedWordsEditorWindow::validate);

	QFontMetrics fontMetrics = QFontMetrics(m_ui->hintTextEdit->font());
	m_ui->hintTextEdit->setMaximumHeight(fontMetrics.lineSpacing() * 4);
	m_ui->hintTextEdit->setMaximumHeight(fontMetrics.lineSpacing() * 4);

	m_ui->listWidget->setStyleSheet("QListWidget::item { border-bottom: 1px solid black; }");
	m_ui->listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

	connect(m_ui->addButton, &QPushButton::clicked, this, ExpectedWordsEditorWindow::onAddItemClicked);

	QIcon warningIcon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
	QPixmap warningPixmap = warningIcon.pixmap(QSize(16, 16));
	m_ui->errorIconLabel->setPixmap(warningPixmap);

	m_ui->buttonBox->button(QDialogButtonBox::Save)->setText("Сохранить");
	m_ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отменить");
	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, ExpectedWordsEditorWindow::onSaveClicked);
	connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, ExpectedWordsEditorWindow::onCancelClicked);

	setNode(expectedWords);
	validate();
}

ExpectedWordsEditorWindow::~ExpectedWordsEditorWindow()
{
	delete m_ui;
}

void ExpectedWordsEditorWindow::onAddItemClicked()
{	
	if (!m_itemEditors.isEmpty() && m_itemEditors.last()->expectedWords().words.trimmed().isEmpty())
	{
		return;
	}

	addItemWidget(Core::ExpectedWords("", 0));

	validate();

	m_ui->listWidget->scrollToBottom();

	m_itemEditors.last()->setFocus();
}

void ExpectedWordsEditorWindow::onSaveClicked()
{
	hide();

	emit accepted(getNode());
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
	QString error;

	Core::ExpectedWordsNode expectedWordsNode = getNode();
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

Core::ExpectedWordsNode ExpectedWordsEditorWindow::getNode() const
{
	QList<Core::ExpectedWords> words;
	for (ExpectedWordEditor* editor : m_itemEditors)
	{
		words.append(editor->expectedWords());
	}

	const bool useCustomHint = m_ui->useHintCheckBox->isChecked();
	if (useCustomHint)
	{
		const QString hint = m_ui->hintTextEdit->toPlainText().trimmed();
		const Core::ExpectedWordsNode expectedWordsNode = Core::ExpectedWordsNode(words, hint);
		return expectedWordsNode;
	}

	const Core::ExpectedWordsNode expectedWordsNode = Core::ExpectedWordsNode(words);
	return expectedWordsNode;
}

void ExpectedWordsEditorWindow::setNode(const Core::ExpectedWordsNode& node)
{
	for (const Core::ExpectedWords& expectedWord : node.expectedWords())
	{
		addItemWidget(expectedWord);
	}

	m_ui->useHintCheckBox->setChecked(node.customHint());
	m_ui->hintLabel->setEnabled(node.customHint());
	m_ui->hintTextEdit->setEnabled(node.customHint());

	if (node.customHint())
	{
		m_ui->hintTextEdit->setText(node.hint());
	}
}
