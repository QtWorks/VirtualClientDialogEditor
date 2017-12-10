#include "arrowlinegraphicsitemmimedata.h"

ArrowLineGraphicsItemMimeData::ArrowLineGraphicsItemMimeData(ArrowLineGraphicsItem* item)
	: m_item(item)
{
}

ArrowLineGraphicsItem* ArrowLineGraphicsItemMimeData::item() const
{
	return m_item;
}
