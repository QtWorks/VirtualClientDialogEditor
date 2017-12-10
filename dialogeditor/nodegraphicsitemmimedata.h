#ifndef NODEGRAPHICSITEMMIMEDATA_H
#define NODEGRAPHICSITEMMIMEDATA_H

#include "nodegraphicsitem.h"
#include <QMimeData>

class NodeGraphicsItemMimeData
	: public QMimeData
{
	Q_OBJECT

public:
	NodeGraphicsItemMimeData(NodeGraphicsItem* item);

	NodeGraphicsItem* item() const;

private:
	NodeGraphicsItem* m_item;
};

#endif // NODEGRAPHICSITEMMIMEDATA_H
