#include "listeditorwidget.h"
#include "ui_listeditorwidget.h"
#include "editablelistitem.h"

ListEditorWidget::ListEditorWidget(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::ListEditorWidget)
	, m_listWidget(m_ui->listWidget)
{
	m_ui->setupUi(this);

	m_listWidget = m_ui->listWidget;
	m_listWidget->setStyleSheet( "QListWidget::item { border-bottom: 1px solid black; }" );

	connect(m_ui->addButton, &QPushButton::clicked, [this]()
	{
		emit itemCreateRequested();
	});
}

ListEditorWidget::~ListEditorWidget()
{
	delete m_ui;
}

void ListEditorWidget::setItems(const QStringList& items)
{
	m_listWidget->clear();

	m_items = items;

	for (const QString& item : items)
	{
		addItemWidget(item);
	}
}

void ListEditorWidget::updateItem(const QString& oldItem, const QString& newItem)
{
	const int index = m_items.indexOf(oldItem);

	m_items[index] = newItem;

	QWidget* itemWidget = m_listWidget->itemWidget(m_listWidget->item(index));
	EditableListItem* editableListItem = dynamic_cast<EditableListItem*>(itemWidget);
	editableListItem->setLabel(newItem);
	bindSignals(editableListItem, newItem);
}

void ListEditorWidget::addItem(const QString& item)
{
	// TODO: same item already exists?
	m_items.append(item);
	addItemWidget(item);
}

void ListEditorWidget::removeItem(const QString& item)
{
	const auto itemIndex = m_items.indexOf(item);

	m_items.removeOne(item);

	delete m_listWidget->takeItem(itemIndex);
}

void ListEditorWidget::addItemWidget(const QString& item)
{
	EditableListItem* editableListItem = new EditableListItem(item, m_ui->listWidget);
	bindSignals(editableListItem, item);

	QListWidgetItem* widget = new QListWidgetItem();
	widget->setSizeHint(QSize(0, 65));

	m_listWidget->addItem(widget);
	m_listWidget->setItemWidget(widget, editableListItem);
}

void ListEditorWidget::bindSignals(EditableListItem* item, QString text)
{
	item->disconnect();

	connect(item, &EditableListItem::editRequested, [this, text]()
	{
		emit itemEditRequested(text);
	});

	connect(item, &EditableListItem::removeRequested, [this, text]()
	{
		emit itemRemoveRequested(text);
	});
}
