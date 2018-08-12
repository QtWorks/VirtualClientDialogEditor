#include "listeditorwidget.h"
#include "ui_listeditorwidget.h"

namespace
{

const QColor c_addedColor = QColor::fromRgb(164, 241, 164);
const QColor c_deletedColor = QColor::fromRgb(255, 153, 153);
const QColor c_updatedColor = QColor::fromRgb(87, 206, 250);

}

CustomFontDelegate::CustomFontDelegate(QObject* parent)
	: QStyledItemDelegate(parent)
{
}

QSize CustomFontDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QSize size = QStyledItemDelegate::sizeHint(option, index);
	size.setHeight(QFontMetrics(option.font).height() * 3);
	return size;
}

ListEditorWidget::ListEditorWidget(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::ListEditorWidget)
{
	m_ui->setupUi(this);

	m_saveButton = m_ui->saveButton;
	m_revertButton = m_ui->revertButton;
	m_revertAllButton = m_ui->revertAllButton;

	connect(m_ui->addButton, &QPushButton::clicked, this, &ListEditorWidget::onAddButtonClicked);
	connect(m_ui->editButton, &QPushButton::clicked, this, &ListEditorWidget::onEditButtonClicked);
	connect(m_ui->removeButton, &QPushButton::clicked, this, &ListEditorWidget::onRemoveButtonClicked);

	connect(m_ui->saveButton, &QPushButton::clicked, this, &ListEditorWidget::onSaveButtonClicked);
	connect(m_ui->revertButton, &QPushButton::clicked, this, &ListEditorWidget::onRevertButtonClicked);
	connect(m_ui->revertAllButton, &QPushButton::clicked, this, &ListEditorWidget::onRevertAllButtonClicked);

	connect(m_ui->addButton, &QPushButton::clicked, m_ui->listWidget, &QAbstractItemView::clearSelection);
	connect(m_ui->editButton, &QPushButton::clicked, m_ui->listWidget, &QAbstractItemView::clearSelection);
	connect(m_ui->removeButton, &QPushButton::clicked, m_ui->listWidget, &QAbstractItemView::clearSelection);
	connect(m_ui->saveButton, &QPushButton::clicked, m_ui->listWidget, &QAbstractItemView::clearSelection);
	connect(m_ui->revertButton, &QPushButton::clicked, m_ui->listWidget, &QAbstractItemView::clearSelection);
	connect(m_ui->revertAllButton, &QPushButton::clicked, m_ui->listWidget, &QAbstractItemView::clearSelection);

	onSelectionChanged();

	m_ui->listWidget->setItemDelegate(new CustomFontDelegate(parent));
	connect(m_ui->listWidget, &QListWidget::itemSelectionChanged, this, &ListEditorWidget::onSelectionChanged);
}

ListEditorWidget::~ListEditorWidget()
{
	delete m_ui;
}

void ListEditorWidget::updateData()
{
	m_ui->listWidget->clear();
	for (const QString& item : items())
	{
		m_ui->listWidget->addItem(item);
	}
}

void ListEditorWidget::updateItem(int index)
{
	const QString item = items()[index];

	QListWidgetItem* listWidgetItem = m_ui->listWidget->item(index);
	listWidgetItem->setText(item);
	listWidgetItem->setBackgroundColor(c_updatedColor);
}

void ListEditorWidget::addItem(const QString& item)
{
	m_ui->listWidget->addItem(item);
	setRowBackground(items().indexOf(item), c_addedColor);
}

void ListEditorWidget::removeItem(const QString& item)
{
	const int index = items().indexOf(item);
	if (itemIsAdded(item))
	{
		delete m_ui->listWidget->takeItem(index);
	}
	else
	{
		setRowBackground(index, c_deletedColor);
	}
}

void ListEditorWidget::showProgressDialog(const QString& title, const QString& label)
{
	Q_ASSERT(m_progressDialog == nullptr);
	m_progressDialog.reset(new QProgressDialog(label, QString(), 0, 0, nullptr));
	m_progressDialog->setWindowTitle(title);
	m_progressDialog->setWindowFlags(m_progressDialog->windowFlags() & ~Qt::WindowCloseButtonHint);
	m_progressDialog->setModal(true);
	m_progressDialog->show();
}

void ListEditorWidget::hideProgressDialog()
{
	Q_ASSERT(m_progressDialog != nullptr);
	m_progressDialog->hide();
	m_progressDialog.reset();
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

void ListEditorWidget::onSaveButtonClicked()
{
	saveChanges();
}

void ListEditorWidget::onRevertButtonClicked()
{
	const QList<QListWidgetItem*> selectedItems = m_ui->listWidget->selectedItems();
	Q_ASSERT(selectedItems.size() == 1);

	revertChanges(selectedItems[0]->text());

	const int index = m_ui->listWidget->row(selectedItems[0]);

	delete m_ui->listWidget->takeItem(index);
	m_ui->listWidget->insertItem(index, items()[index]);
}

void ListEditorWidget::onRevertAllButtonClicked()
{
	revertAllChanges();

	updateData();
}

void ListEditorWidget::onSelectionChanged()
{
	const QList<QListWidgetItem*> selectedItems = m_ui->listWidget->selectedItems();
	m_ui->editButton->setEnabled(selectedItems.size() == 1);
	m_ui->removeButton->setEnabled(selectedItems.size() >= 1);
	m_ui->revertButton->setEnabled(selectedItems.size() == 1 &&
		itemHasChanges(selectedItems.first()->text()) &&
		!itemIsAdded(selectedItems.first()->text())
	);
}

void ListEditorWidget::setRowBackground(int index, const QColor& color)
{
	m_ui->listWidget->item(index)->setBackgroundColor(color);
}
