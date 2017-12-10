#include "nodegraphicsitemmimedata.h"

NodeGraphicsItemMimeData::NodeGraphicsItemMimeData(NodeGraphicsItem* item)
	: m_item(item)
{
}

NodeGraphicsItem* NodeGraphicsItemMimeData::item() const
{
	return m_item;
}
