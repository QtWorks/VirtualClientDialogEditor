#include "arrowlinegraphicsitem.h"
#include "arrowlinegraphicsitemmimedata.h"
#include "nodegraphicsitem.h"
#include "expectedwordsnodegraphicsitem.h"
#include "clientreplicanodegraphicsitem.h"

#include <math.h>

#include <QPen>
#include <QPainter>

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QDrag>
#include <memory>

ArrowLineGraphicsItem::Item::Item(const QPointF& point)
	: m_type(Type::Point)
	, m_point(point)
	, m_lineItem(nullptr)
	, m_isIncomingLink(false)
{
}

ArrowLineGraphicsItem::Item::Item(NodeGraphicsItem* item)
	: m_type(Type::GraphicsItem)
	, m_lineItem(nullptr)
	, m_isIncomingLink(false)
	, m_item(item)
{
}

ArrowLineGraphicsItem::Item::Item(NodeGraphicsItem* item, ArrowLineGraphicsItem* lineItem, bool incomingLink)
	: m_type(Type::GraphicsItem)
	, m_lineItem(lineItem)
	, m_isIncomingLink(incomingLink)
	, m_item(item)
{
	if (m_isIncomingLink)
	{
		m_item->addIncomingLink(m_lineItem);
	}
	else
	{
		m_item->addOutcomingLink(m_lineItem);
	}
}

ArrowLineGraphicsItem::Item::Item(const Item& item)
	: m_type(item.m_type)
	, m_point(item.m_point)
	, m_lineItem(item.m_lineItem)
	, m_isIncomingLink(item.m_isIncomingLink)
	, m_item(item.m_item)
{
}

ArrowLineGraphicsItem::Item::Item(Item&& item)
	: m_type(std::move(item.m_type))
	, m_point(std::move(item.m_point))
	, m_lineItem(std::move(item.m_lineItem))
	, m_isIncomingLink(std::move(item.m_isIncomingLink))
	, m_item(std::move(item.m_item))
{
}

ArrowLineGraphicsItem::Item::~Item()
{
	if (m_type == Type::GraphicsItem && m_lineItem)
	{
		if (m_isIncomingLink)
		{
			m_item->removeIncomingLink(m_lineItem);
		}
		else
		{
			m_item->removeOutcomingLink(m_lineItem);
		}
	}
}

ArrowLineGraphicsItem::Item& ArrowLineGraphicsItem::Item::operator=(const Item& item)
{
	if (this != &item)
	{
		m_type = item.m_type;
		m_point = item.m_point;
		m_lineItem = item.m_lineItem;
		m_isIncomingLink = item.m_isIncomingLink;
		m_item = item.m_item;
	}
	return *this;
}

ArrowLineGraphicsItem::Item& ArrowLineGraphicsItem::Item::operator=(Item&& item)
{
	if (this != &item)
	{
		m_type = std::move(item.m_type);
		item.m_type = Type::Point;

		m_point = std::move(item.m_point);
		item.m_point = QPointF();

		m_lineItem = std::move(item.m_lineItem);
		item.m_lineItem = nullptr;

		m_isIncomingLink = std::move(item.m_isIncomingLink);
		item.m_isIncomingLink = false;

		m_item = std::move(item.m_item);
		item.m_item = nullptr;
	}
	return *this;
}

void ArrowLineGraphicsItem::Item::addIncomingLink(ArrowLineGraphicsItem* link)
{
	if (m_type == Type::GraphicsItem)
	{
		m_item->addIncomingLink(link);
	}
}

void ArrowLineGraphicsItem::Item::addOutcomingLink(ArrowLineGraphicsItem* link)
{
	if (m_type == Type::GraphicsItem)
	{
		m_item->addOutcomingLink(link);
	}
}

void ArrowLineGraphicsItem::Item::removeIncomingLink(ArrowLineGraphicsItem* link)
{
	if (m_type == Type::GraphicsItem)
	{
		m_item->removeIncomingLink(link);
	}
}

void ArrowLineGraphicsItem::Item::removeOutcomingLink(ArrowLineGraphicsItem* link)
{
	if (m_type == Type::GraphicsItem)
	{
		m_item->removeOutcomingLink(link);
	}
}

void ArrowLineGraphicsItem::Item::swap(Item& item)
{
	if (this == &item)
	{
		return;
	}

	std::swap(m_type, item.m_type);
	std::swap(m_point, item.m_point);
	std::swap(m_lineItem, item.m_lineItem);
	std::swap(m_isIncomingLink, item.m_isIncomingLink);
	std::swap(m_item, item.m_item);
}

bool ArrowLineGraphicsItem::Item::collidesWithItem(const Item& item) const
{
	if (m_type == Type::GraphicsItem && item.m_type == Type::GraphicsItem)
	{
		return m_item->collidesWithItem(item.m_item);
	}

	if (m_type == Type::GraphicsItem && item.m_type == Type::Point)
	{
		return m_item->boundingRect().contains(m_item->mapFromScene(item.m_point));
	}

	if (m_type == Type::Point && item.m_type == Type::GraphicsItem)
	{
		return item.m_item->boundingRect().contains(item.m_item->mapFromScene(m_point));
	}

	return m_point == item.m_point;
}

bool ArrowLineGraphicsItem::Item::canChangeTo(const Item& item) const
{
	if (m_type != Type::GraphicsItem || item.m_type != Type::GraphicsItem)
	{
		return true;
	}

	if (m_item == item.m_item ||
		m_item->type() == item.m_item->type())
	{
		return false;
	}

	return true;
}

QPointF ArrowLineGraphicsItem::Item::intersectionPoint(const QLineF& line) const
{
	if (m_type == Type::Point)
	{
		return m_point;
	}

	const auto mapToScene = [this](const QPointF& point) -> QPointF
	{
		Q_ASSERT(m_type == Type::GraphicsItem);
		return m_item->mapToScene(point);
	};

	const QRectF boundingRect = m_item->boundingRect();

	const QVector<QLineF> borders = {
		QLineF(mapToScene(boundingRect.topLeft()), mapToScene(boundingRect.topRight())),
		QLineF(mapToScene(boundingRect.topRight()), mapToScene(boundingRect.bottomRight())),
		QLineF(mapToScene(boundingRect.bottomRight()), mapToScene(boundingRect.bottomLeft())),
		QLineF(mapToScene(boundingRect.bottomLeft()), mapToScene(boundingRect.topLeft()))
	};

	for (const QLineF& border : borders)
	{
		QPointF intersectionPoint;
		QLineF::IntersectType intersectionType = line.intersect(border, &intersectionPoint);
		if (intersectionType == QLineF::BoundedIntersection)
		{
			return intersectionPoint;
		}
	}

	return QPointF();
}

QPointF ArrowLineGraphicsItem::Item::point() const
{
	if (m_type == Type::GraphicsItem)
	{
		return m_item->mapToScene(m_item->boundingRect().center());
	}

	if (m_type == Type::Point)
	{
		return m_point;
	}

	Q_ASSERT(!"Wtf");
	return QPointF();
}

ArrowLineGraphicsItem::Item::Type ArrowLineGraphicsItem::Item::type() const
{
	return m_type;
}

NodeGraphicsItem* ArrowLineGraphicsItem::Item::node() const
{
	Q_ASSERT(m_type == Item::Type::GraphicsItem);
	return m_item;
}

ArrowLineGraphicsItem::ArrowLineGraphicsItem(const QPointF& startPoint, const QPointF& endPoint, QGraphicsItem* parent)
	: ArrowLineGraphicsItem(ArrowLineGraphicsItem::Item(startPoint), ArrowLineGraphicsItem::Item(endPoint), {}, parent)
{
}

ArrowLineGraphicsItem::ArrowLineGraphicsItem(NodeGraphicsItem* startItem, NodeGraphicsItem* endItem,
	const QVector<QPointF>& intermediatePoints, QGraphicsItem* parent)
	: ArrowLineGraphicsItem(Item(startItem, this, false), Item(endItem, this, true), intermediatePoints, parent)
{
}

ArrowLineGraphicsItem::ArrowLineGraphicsItem(const Item& startItem, const Item& endItem, const QVector<QPointF>& intermediatePoints, QGraphicsItem* parent)
	: QGraphicsLineItem(parent)
	, m_isDraggable(false)
	, m_moveMode(MoveMode::NoMove)
	, m_startItem(startItem)
	, m_endItem(endItem)
	, m_intermediatePoints(intermediatePoints)
{
	setFlags(ItemIsMovable | ItemSendsGeometryChanges);
	if (!m_isDraggable)
	{
		setFlag(ItemIsSelectable);
		setFlag(ItemIsFocusable);
	}

	setAcceptHoverEvents(true);

	setZValue(20.0);

	m_startItem.addOutcomingLink(this);
	m_endItem.addIncomingLink(this);

	updatePosition();
}

ArrowLineGraphicsItem::~ArrowLineGraphicsItem()
{
	m_startItem.removeOutcomingLink(this);
	m_endItem.removeIncomingLink(this);
}

int ArrowLineGraphicsItem::type() const
{
	return Type;
}

QRectF ArrowLineGraphicsItem::boundingRect() const
{
	qreal extra = (pen().width() + 20) / 2.0;

	const QPointF p1 = line().p1();
	const QPointF p2 = line().p2();

	return QRectF(p1, QSizeF(p2.x() - p1.x(), p2.y() - p1.y()))
		.normalized()
		.adjusted(-extra, -extra, extra, extra);
}

QPainterPath ArrowLineGraphicsItem::shape() const
{
	QPainterPath path;

	if (m_intermediatePoints.empty())
	{
		path = QGraphicsLineItem::shape();
	}
	else
	{
		for (const QPointF& point : polyline())
		{
			path.lineTo(point);
		}
	}

	if (isSelected())
	{
		path.addEllipse(line().p1(), anchorRadius(), anchorRadius());
		path.addEllipse(line().p2(), anchorRadius(), anchorRadius());
	}

	path.addPolygon(m_arrowHead);

	return path;
}

QVector<QPointF> ArrowLineGraphicsItem::polyline() const
{
	return QVector<QPointF>() << line().p1() << m_intermediatePoints << line().p2();
}

void ArrowLineGraphicsItem::updatePosition()
{
	updatePosition(m_startItem.point(), m_endItem.point(), false);
}

void ArrowLineGraphicsItem::updatePosition(const QPointF& sceneP1, const QPointF& sceneP2, bool updateItems)
{
	if (updateItems)
	{
		Item newStartItem = Item(sceneP1);
		m_startItem.swap(newStartItem);

		Item newEndItem = Item(sceneP2);
		m_endItem.swap(newEndItem);
	}

	const QLineF beginingLine = QLineF(sceneP1, m_intermediatePoints.empty() ? sceneP2 : m_intermediatePoints[0]);
	const QLineF endingLine = QLineF(m_intermediatePoints.empty() ? sceneP1 : m_intermediatePoints.last(), sceneP2);

	const auto adjustedLineBegin = mapFromScene(m_startItem.intersectionPoint(beginingLine));
	const auto adjustedLineEnd = mapFromScene(m_endItem.intersectionPoint(endingLine));

	const auto adjustedLine = QLineF(adjustedLineBegin, adjustedLineEnd);

	setLine(adjustedLine);

	updateArrowHead();
}

void ArrowLineGraphicsItem::setDraggable(bool value)
{
	m_isDraggable = value;

	setFlag(ItemIsSelectable, !m_isDraggable);
	setFlag(ItemIsFocusable, !m_isDraggable);
}

NodeGraphicsItem* ArrowLineGraphicsItem::parentNode() const
{
	Q_ASSERT(isConnectingNodes());
	return m_startItem.node();
}

NodeGraphicsItem* ArrowLineGraphicsItem::childNode() const
{
	Q_ASSERT(isConnectingNodes());
	return m_endItem.node();
}

bool ArrowLineGraphicsItem::isConnectingNodes() const
{
	return m_startItem.type() == Item::Type::GraphicsItem && m_endItem.type() == Item::Type::GraphicsItem;
}

void ArrowLineGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	if (m_startItem.collidesWithItem(m_endItem))
	{
		return;
	}

	painter->setRenderHint(QPainter::Antialiasing);
	painter->setPen(QPen(Qt::black, 1.0, Qt::SolidLine));
	painter->setBrush(Qt::black);

	const QVector<QPointF> points = polyline();
	painter->drawPolyline(points.data(), points.size());
	painter->drawPolygon(m_arrowHead);

	if (isSelected())
	{
		const QColor color = QColor::fromRgb(0x4C, 0xAF, 0x50);
		painter->setPen(color);
		painter->setBrush(color);

		painter->drawEllipse(line().p1(), anchorRadius(), anchorRadius());
		painter->drawEllipse(line().p2(), anchorRadius(), anchorRadius());
	}
}

void ArrowLineGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_isDraggable)
	{
		setCursor(Qt::ClosedHandCursor);
	}
	else
	{
		if (atResizeEllipse(line().p1(), event->pos()))
		{
			m_moveMode = MoveMode::MoveStartPoint;
		}
		else if (atResizeEllipse(line().p2(), event->pos()))
		{
			m_moveMode = MoveMode::MoveEndPoint;
		}
		else
		{
			m_moveMode = MoveMode::MoveWholeArrow;
		}

		setZValue(200.0);
	}

	QGraphicsLineItem::mousePressEvent(event);
}

void ArrowLineGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_moveMode != MoveMode::NoMove)
	{
		if (m_moveMode == MoveMode::MoveWholeArrow)
		{
			// default behaviour is ok
			QGraphicsLineItem::mouseMoveEvent(event);
		}
		else
		{
			const QLineF updatedLine = m_moveMode == MoveMode::MoveStartPoint ?
				QLineF(event->pos(), line().p2()) :
				QLineF(line().p1(), event->pos());
			setLine(updatedLine);
			updateArrowHead();

			setCursor(canMoveTo(event->scenePos()) ? Qt::ArrowCursor : Qt::ForbiddenCursor);
		}
	}
	else if (m_isDraggable)
	{
		const int dragDistance = QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton)).length();
		if (dragDistance < QApplication::startDragDistance())
		{
			return;
		}

		QDrag* drag = new QDrag(QApplication::instance());
		ArrowLineGraphicsItem* draggingItem = new ArrowLineGraphicsItem(m_startItem, m_endItem, {}, parentItem());
		draggingItem->setDraggable(false);
		drag->setMimeData(new ArrowLineGraphicsItemMimeData(draggingItem));

		QPixmap pixmap(boundingRect().size().toSize());
		pixmap.fill(Qt::transparent);

		QPainter painter(&pixmap);
		painter.translate(0, 5);
		paint(&painter, 0, 0);
		painter.end();

		drag->setPixmap(pixmap);

		drag->exec();
		setCursor(Qt::OpenHandCursor);
	}
	else
	{
		m_startItem = Item(mapToScene(line().p1()));
		m_endItem = Item(mapToScene(line().p2()));

		QGraphicsLineItem::mouseMoveEvent(event);
	}
}

void ArrowLineGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_isDraggable)
	{
		setCursor(Qt::OpenHandCursor);
	}
	else if (m_moveMode != MoveMode::NoMove)
	{
		if (m_moveMode == MoveMode::MoveWholeArrow)
		{
			if (event->buttonDownScenePos(Qt::LeftButton) != event->scenePos())
			{
				const bool connectingNodes = isConnectingNodes();
				NodeGraphicsItem* parent = connectingNodes ? parentNode() : nullptr;
				NodeGraphicsItem* child = connectingNodes ? childNode() : nullptr;

				updatePosition(mapToScene(line().p1()), mapToScene(line().p2()), true);

				if (connectingNodes && !isConnectingNodes())
				{
					emit nodesDisconnected(parent, child);
				}
			}
		}
		else
		{
			if (canMoveTo(event->scenePos()))
			{
				Item newItem = itemAtPosition(event->scenePos(), true);
				updatePosition(std::move(newItem));
			}
			else
			{
				Item& changingItem = m_moveMode == MoveMode::MoveStartPoint ? m_startItem : m_endItem;
				updatePosition(std::move(changingItem));
			}
		}

		setSelected(false);

		setZValue(20.0);
		m_moveMode = MoveMode::NoMove;
	}

	QGraphicsLineItem::mouseReleaseEvent(event);
}

void ArrowLineGraphicsItem::keyPressEvent(QKeyEvent* event)
{
	if (!m_isDraggable && event->key() == Qt::Key_Delete)
	{
		emit removeRequested();
	}

	QGraphicsLineItem::keyPressEvent(event);
}

void ArrowLineGraphicsItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
	if (m_isDraggable)
	{
		setCursor(Qt::OpenHandCursor);
	}
	else
	{
		if (isSelected())
		{
			const bool showResizeCursor = atResizeEllipse(line().p1(), event->pos()) ||
				atResizeEllipse(line().p2(), event->pos());

			setCursor(showResizeCursor || !isSelected() ? Qt::CrossCursor : Qt::ArrowCursor);
		}
		else
		{
			setCursor(Qt::ArrowCursor);
		}
	}

	QGraphicsLineItem::hoverMoveEvent(event);
}

void ArrowLineGraphicsItem::updatePosition(Item&& item)
{
	const bool connectingNodes = isConnectingNodes();
	NodeGraphicsItem* parent = connectingNodes ? parentNode() : nullptr;
	NodeGraphicsItem* child = connectingNodes ? childNode() : nullptr;

	if (m_moveMode == MoveMode::MoveStartPoint)
	{
		m_startItem.swap(item);
	}
	else if (m_moveMode == MoveMode::MoveEndPoint)
	{
		m_endItem.swap(item);
	}
	else
	{
		Q_ASSERT(!"updatePosition(const Item& item) must be called only while resizing");
		return;
	}

	if (!connectingNodes && isConnectingNodes())
	{
		emit nodesConnected(parentNode(), childNode());
	}
	else if (connectingNodes && !isConnectingNodes())
	{
		emit nodesDisconnected(parent, child);
	}

	updatePosition();
}

void ArrowLineGraphicsItem::updateArrowHead()
{
	const QLineF arrowLine = m_intermediatePoints.empty() ? line() : QLineF(m_intermediatePoints.last(), line().p2());

	double angle = ::acos(arrowLine.dx() / arrowLine.length());
	if (arrowLine.dy() >= 0)
	{
		angle = (M_PI * 2) - angle;
	}

	const qreal arrowSize = 10;

	const QPointF arrowEnd = line().p2();
	QPointF arrowP1 = arrowEnd - QPointF(sin(angle + M_PI / 3) * arrowSize, cos(angle + M_PI / 3) * arrowSize);
	QPointF arrowP2 = arrowEnd - QPointF(sin(angle + M_PI - M_PI / 3) * arrowSize, cos(angle + M_PI - M_PI / 3) * arrowSize);

	m_arrowHead.clear();
	m_arrowHead << arrowEnd << arrowP1 << arrowP2;
}

QList<QGraphicsItem*> ArrowLineGraphicsItem::nodesAtPosition(const QPointF& position) const
{
	auto items = scene()->items(position);

	auto removedItems = std::remove_if(items.begin(), items.end(), [](QGraphicsItem* item)
	{
		return item->type() != ExpectedWordsNodeGraphicsItem::Type &&
			item->type() != ClientReplicaNodeGraphicsItem::Type;
	});

	items.erase(removedItems, items.end());

	return items;
}

ArrowLineGraphicsItem::Item ArrowLineGraphicsItem::itemAtPosition(const QPointF& position, const bool link)
{
	const auto nodes = nodesAtPosition(position);
	if (nodes.isEmpty())
	{
		return Item(position);
	}

	NodeGraphicsItem* node = static_cast<NodeGraphicsItem*>(nodes.first());
	return link ? Item(node, this, m_moveMode == MoveMode::MoveEndPoint) : Item(node);
}

bool ArrowLineGraphicsItem::canMoveTo(const QPointF& position)
{
	Q_ASSERT(m_moveMode == MoveMode::MoveStartPoint || m_moveMode == MoveMode::MoveEndPoint);

	const Item movingItem = itemAtPosition(position, false);
	const Item& notMovingItem = m_moveMode == MoveMode::MoveStartPoint ? m_endItem : m_startItem;

	if (movingItem.m_type == Item::Type::Point)
	{
		return true;
	}

	if (notMovingItem.m_type == Item::Type::GraphicsItem)
	{
		// same item
		if (notMovingItem.m_item == movingItem.m_item ||
			notMovingItem.m_item->type() == movingItem.m_item->type())
		{
			return false;
		}
	}

	const bool isIncomingLink = m_moveMode == MoveMode::MoveEndPoint;
	if (isIncomingLink && movingItem.m_item->incomingLinks().size() >= 1)
	{
		return false;
	}

	if (!isIncomingLink && movingItem.m_item->outcomingLinks().size() >= 1)
	{
		return false;
	}

	return true;
}

int ArrowLineGraphicsItem::anchorRadius()
{
	return 3;
}

bool ArrowLineGraphicsItem::atResizeEllipse(const QPointF& ellipseCenter, const QPointF& point)
{
	typedef std::unique_ptr<QGraphicsEllipseItem> QGraphicsEllipseItemPtr;
	const auto makeEllipseItem = [](const QPointF& point) -> QGraphicsEllipseItemPtr
	{
		int x = point.x() - anchorRadius();
		int y = point.y() - anchorRadius();
		return QGraphicsEllipseItemPtr(new QGraphicsEllipseItem(x, y, anchorRadius() * 2, anchorRadius() * 2));
	};

	auto ellipseItem = makeEllipseItem(ellipseCenter);
	return ellipseItem->contains(point);
}
