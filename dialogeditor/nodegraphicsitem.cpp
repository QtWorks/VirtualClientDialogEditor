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
	: m_width(0)
	, m_height(0)
	, m_position(pos())
	, m_resizing(false)
{
	setFlags(ItemIsMovable | ItemSendsGeometryChanges);

	setProperties(properties);
	setAcceptHoverEvents(true);

	setParent(parent);

	resizeToMinimal();
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

void NodeGraphicsItem::resize(qreal width, qreal height)
{
	if (m_width == width && m_height == height)
	{
		return;
	}

	LOG << "Resize from " << ARG(m_width) << ARG(m_height) << " to " << ARG(width) << ARG(height);

	prepareGeometryChange();
	m_width = width;
	m_height = height;
}

void NodeGraphicsItem::resizeToMinimal()
{
	resize(minWidth(), minHeight());
}

QPointF NodeGraphicsItem::position() const
{
	return m_position;
}

void NodeGraphicsItem::setPosition(const QPointF& position)
{
	m_position = position;
	setPos(m_position);
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
}

QList<ArrowLineGraphicsItem*> NodeGraphicsItem::outcomingLinks() const
{
	return m_outcomingLinks;
}

PhaseGraphicsItem* NodeGraphicsItem::getPhase() const
{
	return m_phase;
}

void NodeGraphicsItem::setPhase(PhaseGraphicsItem* phase)
{
	m_phase = phase;
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

	if (m_properties & HighlightInvalid && !data()->validate())
	{
		painter->setBrush(invalidBrush());
	}
	else
	{
		painter->setBrush(getHeaderBrush());
	}

	const int headerHeight = painter->fontMetrics().height() + padding() * 2;
	const int roundRadius = 6;

	const QRectF headerRect = QRectF(0, 0, m_width, headerHeight);
	QPainterPath headerRectPath;
	headerRectPath.setFillRule(Qt::WindingFill);
	headerRectPath.addRoundedRect(headerRect, roundRadius, roundRadius);
	headerRectPath.addRect(QRectF(headerRect.left(), headerRect.bottom() - roundRadius, roundRadius, roundRadius));
	headerRectPath.addRect(QRectF(headerRect.right() - roundRadius, headerRect.bottom() - roundRadius, roundRadius, roundRadius));
	painter->drawPath(headerRectPath.simplified());

	QRectF adjustedHeaderRect = headerRect.adjusted(+padding(), +padding(), -padding(), -padding());
	const QString elidedHeaderText = elideText(painter->fontMetrics(), getHeaderText(), adjustedHeaderRect.width(), adjustedHeaderRect.height());
	painter->drawText(adjustedHeaderRect, Qt::AlignLeft, elidedHeaderText);

	painter->setBrush(getContentBrush());

	const QRectF contentRect = QRectF(0, headerHeight, m_width, m_height - headerHeight);
	QPainterPath contentRectPath;
	contentRectPath.setFillRule(Qt::WindingFill);
	contentRectPath.addRoundedRect(contentRect, roundRadius, roundRadius);
	contentRectPath.addRect(QRectF(contentRect.left(), contentRect.top(), roundRadius, roundRadius));
	contentRectPath.addRect(QRectF(contentRect.right() - roundRadius, contentRect.top(), roundRadius, roundRadius));
	painter->drawPath(contentRectPath.simplified());

	QRectF adjustedContentRect = contentRect.adjusted(+padding(), +padding(), -padding(), -padding());
	const QString elidedContentText = elideText(painter->fontMetrics(), getContentText(), adjustedContentRect.width(), adjustedContentRect.height());
	painter->drawText(adjustedContentRect, Qt::AlignLeft, elidedContentText);
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

		const qreal width = std::max(eventPosition.x(), minWidth());
		const qreal height = std::max(eventPosition.y(), minHeight());
		resize(width, height);
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
		draggingItem->setProperties(Resizable | Editable | Removable | HighlightInvalid);
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

	const QPointF oldPosition = m_position;
	const QPointF newPosition = pos();

	if (oldPosition != newPosition)
	{
		m_position = newPosition;
		emit positionChanged(oldPosition, newPosition);
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
		setCursor(atResizeRect ? Qt::SizeFDiagCursor : Qt::OpenHandCursor);
	}

	QGraphicsItem::hoverMoveEvent(event);
}

QVariant NodeGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
	if (change == ItemSelectedChange)
	{
		const bool selected = value.toBool();

		setZValue(selected ? zValue() * 10 : zValue() / 10);

		emit selectionChanged(selected);
	}

	const bool isPositionChanged = change == ItemPositionChange || change == ItemPositionHasChanged;
	const bool isSizeChanging = m_resizing && (change == ItemCursorChange || change == ItemCursorHasChanged);
	if (isPositionChanged || isSizeChanging)
	{
		trackNodes();
	}

	return QGraphicsItem::itemChange(change, value);
}

int NodeGraphicsItem::padding()
{
	return 8;
}

qreal NodeGraphicsItem::minHeight() const
{
	static const qreal s_minHeight = qApp->fontMetrics().height() * 2 + padding() * 4;
	return s_minHeight;
}

qreal NodeGraphicsItem::minWidth() const
{
	static const qreal s_minWidth = 177.0;
	return s_minWidth;
}

QRectF NodeGraphicsItem::outlineRect() const
{
	return QRect(0, 0, m_width, m_height);
}

QBrush NodeGraphicsItem::invalidBrush() const
{
	return QColor::fromRgb(0xFF5252);
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

QBrush NodeGraphicsItem::getContentBrush() const
{
	return QBrush(Qt::white);
}
