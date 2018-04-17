#include "listeditorwidget.h"
#include "ui_listeditorwidget.h"

CustomFontDelegate::CustomFontDelegate(QObject* parent)
	: QStyledItemDelegate(parent)
{
}

QSize CustomFontDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QFontMetrics metrics(option.font);

	QSize size = QStyledItemDelegate::sizeHint(option, index);
	size.setHeight(metrics.height() * 3);

	return size;
}

ListEditorWidget::ListEditorWidget(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::ListEditorWidget)
{
	m_ui->setupUi(this);

	CustomFontDelegate* delegate = new CustomFontDelegate(parent);
	m_ui->listWidget->setItemDelegate(delegate);

	connect(m_ui->addButton, &QPushButton::clicked, this, &ListEditorWidget::onAddButtonClicked);
	connect(m_ui->editButton, &QPushButton::clicked, this, &ListEditorWidget::onEditButtonClicked);
	connect(m_ui->removeButton, &QPushButton::clicked, this, &ListEditorWidget::onRemoveButtonClicked);
	onSelectionChanged();

	connect(m_ui->listWidget, &QListWidget::itemSelectionChanged, this, &ListEditorWidget::onSelectionChanged);
}

ListEditorWidget::~ListEditorWidget()
{
	delete m_ui;
}

void ListEditorWidget::setItems(const QStringList& items)
{
	m_ui->listWidget->clear();

	QStringList uniqueItems = items;
	const int duplicatesCount = uniqueItems.removeDuplicates();
	Q_ASSERT(duplicatesCount == 0);
	m_items = uniqueItems;

	for (const QString& item : items)
	{
		m_ui->listWidget->addItem(item);
	}
}

void ListEditorWidget::updateItem(const QString& oldItem, const QString& newItem)
{
	const int index = m_items.indexOf(oldItem);

	m_items[index] = newItem;

	QListWidgetItem* listWidgetItem = m_ui->listWidget->item(index);
	listWidgetItem->setText(newItem);
}

void ListEditorWidget::addItem(const QString& item)
{
	m_items.append(item);

	m_ui->listWidget->addItem(item);
}

void ListEditorWidget::removeItem(const QString& item)
{
	const auto itemIndex = m_items.indexOf(item);

	m_items.removeOne(item);

	delete m_ui->listWidget->takeItem(itemIndex);
}

void ListEditorWidget::onAddButtonClicked()
{
	emit itemCreateRequested();
}

void ListEditorWidget::onEditButtonClicked()
{
	const QList<QListWidgetItem*> selectedItems = m_ui->listWidget->selectedItems();
	Q_ASSERT(selectedItems.size() == 1);


	emit itemEditRequested(selectedItems.first()->text());
}

void ListEditorWidget::onRemoveButtonClicked()
{
	const QList<QListWidgetItem*> selectedItems = m_ui->listWidget->selectedItems();
	Q_ASSERT(selectedItems.size() >= 1);

	QStringList items;
	for (const QListWidgetItem* item : selectedItems)
	{
		items << item->text();
	}

	emit itemsRemoveRequested(items);
}

void ListEditorWidget::onSelectionChanged()
{
	const int selectedItemsCount = m_ui->listWidget->selectedItems().size();
	m_ui->editButton->setEnabled(selectedItemsCount == 1);
	m_ui->removeButton->setEnabled(selectedItemsCount >= 1);
}
