#include "nodegraphicsitem.h"
#include "arrowlinegraphicsitem.h"
#include "fontmetricsheighteliding.h"
#include "nodegraphicsitemmimedata.h"

#include <QApplication>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QDrag>
#include <QMimeData>

NodeGraphicsItem::NodeGraphicsItem(Properties properties, QObject* parent)
	: m_width(minWidth())
	, m_height(minHeight())
	, m_resizing(false)
{
	setFlags(ItemIsMovable | ItemSendsGeometryChanges);

	setProperties(properties);
	setAcceptHoverEvents(true);

	setParent(parent);
}

NodeGraphicsItem::~NodeGraphicsItem()
{
	for (ArrowLineGraphicsItem* link : m_incomingLinks)
	{
		delete link;
	}

	for (ArrowLineGraphicsItem* link : m_outcomingLinks)
	{
		delete link;
	}
}

void NodeGraphicsItem::setProperties(Properties properties)
{
	if (properties.testFlag(Resizable) && properties.testFlag(Draggable))
	{
		Q_ASSERT(!"Wrong combination of flags");
	}

	m_properties = properties;

	if (m_properties.testFlag(Removable))
	{
		setFlag(ItemIsFocusable, true);
	}

	if (!m_properties.testFlag(Draggable))
	{
		setFlag(ItemIsSelectable, true);
	}
}

void NodeGraphicsItem::addIncomingLink(ArrowLineGraphicsItem* link)
{
	m_incomingLinks.append(link);
}

void NodeGraphicsItem::removeIncomingLink(ArrowLineGraphicsItem* link)
{
	int index = m_incomingLinks.indexOf(link);
	if (index != -1)
	{
		m_incomingLinks.removeAt(index);
	}
}

QList<ArrowLineGraphicsItem*> NodeGraphicsItem::incomingLinks() const
{
	return m_incomingLinks;
}

void NodeGraphicsItem::addOutcomingLink(ArrowLineGraphicsItem* link)
{
	m_outcomingLinks.append(link);
}

void NodeGraphicsItem::removeOutcomingLink(ArrowLineGraphicsItem* link)
{
	int index = m_outcomingLinks.indexOf(link);
	if (index != -1)
	{
		m_outcomingLinks.removeAt(index);
	}
	else
	{
		LOG << "NodeGraphicsItem::removeOutcomingLink link not found";
	}
}

QList<ArrowLineGraphicsItem*> NodeGraphicsItem::outcomingLinks() const
{
	return m_outcomingLinks;
}

bool NodeGraphicsItem::atResizeRect(const QPointF& position, const QRectF& rect)
{
	const QPointF bottomRight = rect.bottomRight();

	return
		(position.x() >= (bottomRight.x() - 7) && position.x() <= bottomRight.x()) &&
		(position.y() >= (bottomRight.y() - 7) && position.y() <= bottomRight.y());
}

void NodeGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
	painter->setRenderHint(QPainter::Antialiasing);

	QPen pen = QPen(Qt::black,
		isSelected() ? 2.0 : 1.0,
		isSelected() ? Qt::DotLine : Qt::SolidLine);
	painter->setPen(pen);

	painter->setBrush(getBrush());

	QRectF rect = outlineRect();
	painter->drawRoundRect(rect, roundness(rect.width()), roundness(rect.height()));

	// TODO: eliding in case of new lines
	QRectF contentRect = rect.adjusted(+padding(), +padding(), -padding(), -padding());
	const QString elidedText = elideText(painter->fontMetrics(), getText(), contentRect.width(), contentRect.height());
	painter->drawText(contentRect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);
}

QRectF NodeGraphicsItem::boundingRect() const
{
	static const int s_margin = 1;
	return outlineRect().adjusted(-s_margin, -s_margin, +s_margin, +s_margin);
}

void NodeGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_properties & Resizable)
	{
		m_resizing = atResizeRect(event->scenePos(), mapRectToScene(boundingRect()));
	}
	else if (m_properties & Draggable)
	{
		setCursor(Qt::ClosedHandCursor);
	}

	QGraphicsItem::mousePressEvent(event);
}

void NodeGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_properties & Resizable && m_resizing)
	{
		const QPointF eventPosition = mapFromScene(event->scenePos());

		prepareGeometryChange();
		m_width = std::max(eventPosition.x(), minWidth());
		m_height = std::max(eventPosition.y(), minHeight());
	}
	else if (m_properties & Draggable)
	{
		const int dragDistance = QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton)).length();
		if (dragDistance < QApplication::startDragDistance())
		{
			return;
		}

		QDrag* drag = new QDrag(this);
		NodeGraphicsItem* draggingItem = clone();
		draggingItem->setProperties(Resizable | Editable | Removable);
		drag->setMimeData(new NodeGraphicsItemMimeData(draggingItem));

		QPixmap pixmap(boundingRect().size().toSize());
		pixmap.fill(Qt::transparent);

		QPainter painter(&pixmap);
		paint(&painter, 0, 0);
		painter.end();

		drag->setPixmap(pixmap);

		drag->exec();
		setCursor(Qt::OpenHandCursor);
	}
	else
	{
		QGraphicsItem::mouseMoveEvent(event);
	}
}

void NodeGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_properties & Resizable && m_resizing)
	{
		setSelected(false);
		m_resizing = false;
	}
	else if (m_properties & Draggable)
	{
		setCursor(Qt::OpenHandCursor);
	}

	QGraphicsItem::mouseReleaseEvent(event);
}

void NodeGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_properties & Editable)
	{
		showNodeEditor();
	}

	QGraphicsItem::mouseDoubleClickEvent(event);
}

void NodeGraphicsItem::keyPressEvent(QKeyEvent* event)
{
	if (m_properties & Removable && event->key() == Qt::Key_Delete)
	{
		emit removeRequested();
	}

	QGraphicsItem::keyPressEvent(event);
}

void NodeGraphicsItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
	if (m_properties & Draggable)
	{
		setCursor(Qt::OpenHandCursor);
	}
	else if (m_properties & Resizable)
	{
		const QPointF eventScenePos = event->scenePos();

		const bool atResizeRect = NodeGraphicsItem::atResizeRect(eventScenePos, sceneBoundingRect());
		setCursor(atResizeRect ? Qt::SizeFDiagCursor : Qt::ArrowCursor);
	}

	QGraphicsItem::hoverMoveEvent(event);
}

QVariant NodeGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
	const bool isPositionChanged = change == ItemPositionChange || change == ItemPositionHasChanged;
	const bool isSizeChanging = m_resizing && (change == ItemCursorChange || change == ItemCursorHasChanged);

	if (isPositionChanged || isSizeChanging)
	{
		trackNodes();
	}

	return QGraphicsItem::itemChange(change, value);
}

int NodeGraphicsItem::roundness(double size)
{
	static const int s_diameter = 12;
	return 100 * s_diameter / static_cast<int>(size);
}

int NodeGraphicsItem::padding()
{
	return 8;
}

qreal NodeGraphicsItem::minHeight()
{
	static const qreal s_minHeight = qApp->fontMetrics().height() * 2 + padding() * 2;
	return s_minHeight;
}

qreal NodeGraphicsItem::minWidth()
{
	static const qreal s_minWidth = 177.0;
	return s_minWidth;
}

QRectF NodeGraphicsItem::outlineRect() const
{
	return QRect(0, 0, m_width, m_height);
}

void NodeGraphicsItem::trackNodes()
{
	for (ArrowLineGraphicsItem* link : m_incomingLinks)
	{
		link->updatePosition();
	}

	for (ArrowLineGraphicsItem* link : m_outcomingLinks)
	{
		link->updatePosition();
	}
}