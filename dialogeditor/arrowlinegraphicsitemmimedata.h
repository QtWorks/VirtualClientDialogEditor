#ifndef ARROWLINEGRAPHICSITEMMIMEDATA_H
#define ARROWLINEGRAPHICSITEMMIMEDATA_H

#include "arrowlinegraphicsitem.h"
#include <QMimeData>

class ArrowLineGraphicsItemMimeData
	: public QMimeData
{
	Q_OBJECT

public:
	ArrowLineGraphicsItemMimeData(ArrowLineGraphicsItem* item);

	ArrowLineGraphicsItem* item() const;

private:
	ArrowLineGraphicsItem* m_item;
};

#endif // ARROWLINEGRAPHICSITEMMIMEDATA_H
