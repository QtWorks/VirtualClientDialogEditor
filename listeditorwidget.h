#ifndef LISTEDITORWIDGET_H
#define LISTEDITORWIDGET_H

#include <QWidget>
#include <QStringList>
#include <QListWidget>
#include <QStyledItemDelegate>

namespace Ui {
class ListEditorWidget;
}

class CustomFontDelegate
	: public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit CustomFontDelegate(QObject* parent = 0);

	virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

class ListEditorWidget
	: public QWidget
{
	Q_OBJECT

public:
	explicit ListEditorWidget(QWidget* parent = 0);
	~ListEditorWidget();

	void setItems(const QStringList& items);
	void updateItem(const QString& oldItem, const QString& newItem);
	void addItem(const QString& item);
	void removeItem(const QString& item);

signals:
	void itemEditRequested(QString item);
	void itemsRemoveRequested(QStringList items);
	void itemCreateRequested();

private slots:
	void onAddButtonClicked();
	void onEditButtonClicked();
	void onRemoveButtonClicked();
	void onSelectionChanged();

private:
	Ui::ListEditorWidget* m_ui;

	QStringList m_items;
};

#endif // LISTEDITORWIDGET_H
