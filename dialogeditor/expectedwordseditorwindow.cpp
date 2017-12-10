#include "expectedwordseditorwindow.h"
#include "ui_expectedwordseditorwindow.h"
#include "expectedwordeditor.h"

ExpectedWordsEditorWindow::ExpectedWordsEditorWindow(const QStringList& expectedWords, QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::ExpectedWordsEditorWindow)
{
	m_ui->setupUi(this);

	m_ui->listWidget->setStyleSheet( "QListWidget::item { border-bottom: 1px solid black; }" );
	m_ui->listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

	for (const QString& expectedWord : expectedWords)
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
	if (m_items.isEmpty() || !m_items.last()->text().isEmpty())
	{
		addItemWidget("");

		updateControls();

		m_ui->listWidget->scrollToBottom();

		m_items.last()->setFocus();
	}
}

void ExpectedWordsEditorWindow::onSaveClicked()
{
	hide();

	QStringList expectedWords;
	for (const auto& item : m_items)
	{
		expectedWords.append(item->text().trimmed());
	}

	emit accepted(expectedWords);
}

void ExpectedWordsEditorWindow::onCancelClicked()
{
	hide();
	emit rejected();
}

void ExpectedWordsEditorWindow::addItemWidget(const QString& item)
{
	ExpectedWordEditor* editorItem = new ExpectedWordEditor(item, m_ui->listWidget);
	m_items.append(editorItem);

	connect(editorItem, &ExpectedWordEditor::changed, [this]()
	{
		updateControls();
	});

	connect(editorItem, &ExpectedWordEditor::removed, [this, editorItem]()
	{
		int itemIndex = m_items.indexOf(editorItem);
		Q_ASSERT(m_ui->listWidget->takeItem(itemIndex));
		m_items.removeAt(itemIndex);

		updateControls();
	});

	QListWidgetItem* widget = new QListWidgetItem();
	widget->setSizeHint(QSize(editorItem->width(), editorItem->maximumHeight()));

	m_ui->listWidget->addItem(widget);
	m_ui->listWidget->setItemWidget(widget, editorItem);
}

void ExpectedWordsEditorWindow::updateControls()
{
	const bool saveAllowed = !m_items.isEmpty() &&
		std::none_of(m_items.begin(), m_items.end(), [](ExpectedWordEditor* editor)
		{
			return editor->text().trimmed().isEmpty();
		});

	m_ui->errorIconLabel->setVisible(!saveAllowed);
	m_ui->errorTextLabel->setVisible(!saveAllowed);
	m_ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(saveAllowed);
}
