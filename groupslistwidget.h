#pragma once

#include <QListWidget>
#include "core/client.h"

class GroupsListWidget
	: public QListWidget
{
public:
	GroupsListWidget(QWidget* parent = nullptr);

	void setItems(const QList<Core::Group>& groups);
	void setCheckedItems(const QList<QString>& groups);
	QList<QString> checkedItems();
};
