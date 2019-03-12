#include "listeditorwidget.h"
#include "ui_listeditorwidget.h"

#include <QMessageBox>

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

	connect(m_ui->addButton, &QPushButton::clicked, this, &ListEditorWidget::onAddButtonClicked);
	connect(m_ui->addButton, &QPushButton::clicked, m_ui->listWidget, &QAbstractItemView::clearSelection);

	connect(m_ui->editButton, &QPushButton::clicked, this, &ListEditorWidget::onEditButtonClicked);
	connect(m_ui->editButton, &QPushButton::clicked, m_ui->listWidget, &QAbstractItemView::clearSelection);

	connect(m_ui->removeButton, &QPushButton::clicked, this, &ListEditorWidget::onRemoveButtonClicked);
	connect(m_ui->removeButton, &QPushButton::clicked, m_ui->listWidget, &QAbstractItemView::clearSelection);

	onSelectionChanged();

	m_ui->listWidget->setItemDelegate(new CustomFontDelegate(parent));
	connect(m_ui->listWidget, &QListWidget::itemSelectionChanged, this, &ListEditorWidget::onSelectionChanged);
	connect(m_ui->listWidget, &QListWidget::itemDoubleClicked, this, &ListEditorWidget::onEditButtonClicked);

	m_ui->additionalButtonsWidget->hide();
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

void ListEditorWidget::showProgressDialog(const QString& title, const QString& label)
{
	if (m_progressDialog)
	{
		hideProgressDialog();
	}

	m_progressDialog.reset(new QProgressDialog(label, QString(), 0, 0, nullptr));
	m_progressDialog->setWindowTitle(title);
	m_progressDialog->setWindowFlags(m_progressDialog->windowFlags() & ~Qt::WindowCloseButtonHint);
	m_progressDialog->setModal(true);
	m_progressDialog->show();
}

void ListEditorWidget::hideProgressDialog()
{
	if (!m_progressDialog)
	{
		return;
	}

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

	QMessageBox messageBox(QMessageBox::Question,
		"Удаление записей",
		"Вы действительно хотите удалить " + QString(items.size() > 1 ? "выбранные записи" : "выбранную запись") + "?",
		QMessageBox::Yes | QMessageBox::No,
		this);
	messageBox.setButtonText(QMessageBox::Yes, tr("Да"));
	messageBox.setButtonText(QMessageBox::No, tr("Нет"));

	const int answer = messageBox.exec();
	if (answer != QMessageBox::Yes)
	{
		return;
	}

	removeItems(items);
}

void ListEditorWidget::onSelectionChanged()
{
	const QList<QListWidgetItem*> selectedItems = m_ui->listWidget->selectedItems();
	m_ui->editButton->setEnabled(selectedItems.size() == 1);
	m_ui->removeButton->setEnabled(selectedItems.size() >= 1);
}
