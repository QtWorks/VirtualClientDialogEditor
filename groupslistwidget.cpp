#include "groupslistwidget.h"

GroupsListWidget::GroupsListWidget(QWidget* parent)
	: QListWidget(parent)
{
}

void GroupsListWidget::setItems(const QList<Core::Group>& groups)
{
	for (const auto& group : groups)
	{
		QListWidgetItem* item = new QListWidgetItem(group.name, this);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setData(Qt::UserRole, group.id);
	}
}

void GroupsListWidget::setCheckedItems(const QList<QString>& checkedGroups)
{
	for (int i = 0; i < count(); ++i)
	{
		QListWidgetItem* listWidgetItem = item(i);
		const bool checked = checkedGroups.contains(listWidgetItem->data(Qt::UserRole).toString());
		listWidgetItem->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
	}
}

QList<QString> GroupsListWidget::checkedItems()
{
	QList<QString> result;

	for (int i = 0; i < count(); ++i)
	{
		QListWidgetItem* listWidgetItem = item(i);
		if (listWidgetItem->checkState() == Qt::Checked)
		{
			result.append(listWidgetItem->data(Qt::UserRole).toString());
		}
	}

	return result;
}
