#ifndef LISTEDITORWIDGET_H
#define LISTEDITORWIDGET_H

#include <QWidget>
#include <QStringList>
#include <QListWidget>

namespace Ui {
class ListEditorWidget;
}

class EditableListItem;

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
	void itemRemoveRequested(QString item);
	void itemCreateRequested();

private:
	void addItemWidget(const QString& item);
	void bindSignals(EditableListItem* item, QString text);

private:
	Ui::ListEditorWidget* m_ui;

	QStringList m_items;
	QListWidget* m_listWidget;
};

#endif // LISTEDITORWIDGET_H
