#ifndef EDITABLELISTITEM_H
#define EDITABLELISTITEM_H

#include <QListWidgetItem>

namespace Ui {
class EditableListItem;
}

class EditableListItem
	: public QWidget
{
	Q_OBJECT

public:
	explicit EditableListItem(const QString& label, QWidget* parent = 0);
	~EditableListItem();

	void setLabel(const QString& label);

signals:
	void editRequested();
	void removeRequested();

private:
	Ui::EditableListItem* m_ui;
};

#endif // EDITABLELISTITEM_H
