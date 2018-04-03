#include "expectedwordseditorwindow.h"
#include "ui_expectedwordseditorwindow.h"
#include "expectedwordeditor.h"

ExpectedWordsEditorWindow::ExpectedWordsEditorWindow(const Core::ExpectedWordsNode& expectedWords, QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::ExpectedWordsEditorWindow)
{
	m_ui->setupUi(this);

	m_ui->listWidget->setStyleSheet( "QListWidget::item { border-bottom: 1px solid black; }" );
	m_ui->listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

	for (const Core::ExpectedWords& expectedWord : expectedWords.expectedWords)
	{
		addItemWidget(expectedWord);
	}

	connect(m_ui->addButton, &QPushButton::clicked, this, ExpectedWordsEditorWindow::onAddItemClicked);

	QIcon warningIcon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
	QPixmap warningPixmap = warningIcon.pixmap(QSize(16, 16));
	m_ui->errorIconLabel->setPixmap(warningPixmap);

	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, ExpectedWordsEditorWindow::onSaveClicked);
	connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, ExpectedWordsEditorWindow::onCancelClicked);

	updateControls();
}

ExpectedWordsEditorWindow::~ExpectedWordsEditorWindow()
{
	delete m_ui;
}

void ExpectedWordsEditorWindow::onAddItemClicked()
{	
	if (m_itemEditors.isEmpty() || !m_itemEditors.last()->text().isEmpty())
	{
		addItemWidget(Core::ExpectedWords("", 0));

		updateControls();

		m_ui->listWidget->scrollToBottom();

		m_itemEditors.last()->setFocus();
	}
}

void ExpectedWordsEditorWindow::onSaveClicked()
{
	hide();

	QList<Core::ExpectedWords> expectedWords;
	for (const auto& item : m_itemEditors)
	{
		expectedWords << Core::ExpectedWords(item->text().trimmed(), 0);
	}

	emit accepted(expectedWords);
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
		updateControls();
	});

	connect(editorItem, &ExpectedWordEditor::removed, [this, editorItem]()
	{
		int itemIndex = m_itemEditors.indexOf(editorItem);
		Q_ASSERT(m_ui->listWidget->takeItem(itemIndex));
		m_itemEditors.removeAt(itemIndex);

		updateControls();
	});

	QListWidgetItem* widget = new QListWidgetItem();
	widget->setSizeHint(QSize(editorItem->width(), editorItem->maximumHeight()));

	m_ui->listWidget->addItem(widget);
	m_ui->listWidget->setItemWidget(widget, editorItem);
}

void ExpectedWordsEditorWindow::updateControls()
{
	const bool saveAllowed = !m_itemEditors.isEmpty() &&
		std::none_of(m_itemEditors.begin(), m_itemEditors.end(), [](ExpectedWordEditor* editor)
		{
			return editor->text().trimmed().isEmpty();
		});

	m_ui->errorIconLabel->setVisible(!saveAllowed);
	m_ui->errorTextLabel->setVisible(!saveAllowed);
	m_ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(saveAllowed);
}
