#ifndef NODEGRAPHICSITEM_H
#define NODEGRAPHICSITEM_H

#include "logger.h"
#include <QGraphicsObject>
#include <QBrush>

class ArrowLineGraphicsItem;

class NodeGraphicsItem
	: public QGraphicsObject
{
	Q_OBJECT

public:
	enum Property
	{
		NoProperties = 0x0,
		Resizable = 0x1,
		Editable = 0x2,
		Removable = 0x4,
		Draggable = 0x8
	};
	Q_DECLARE_FLAGS(Properties, Property)

	NodeGraphicsItem(Properties properties, QObject* parent = 0);
	~NodeGraphicsItem();

	void setProperties(Properties properties);

	void addIncomingLink(ArrowLineGraphicsItem* link);
	void removeIncomingLink(ArrowLineGraphicsItem* link);
	QList<ArrowLineGraphicsItem*> incomingLinks() const;

	void addOutcomingLink(ArrowLineGraphicsItem* link);
	void removeOutcomingLink(ArrowLineGraphicsItem* link);
	QList<ArrowLineGraphicsItem*> outcomingLinks() const;

	static bool atResizeRect(const QPointF& position, const QRectF& rect);

	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/ = Q_NULLPTR) override;
	virtual QRectF boundingRect() const override;
	virtual void showNodeEditor() = 0;

signals:
	void removeRequested();

public:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
	virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;

	virtual void dragEnterEvent(QGraphicsSceneDragDropEvent*) override { LOG << "NodeGraphicsItem::dragEnterEvent"; }
	virtual void dragMoveEvent(QGraphicsSceneDragDropEvent*) override { LOG << "NodeGraphicsItem::dragMoveEvent"; }
	virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent*) override { LOG << "NodeGraphicsItem::dragLeaveEvent"; }
	virtual void dropEvent(QGraphicsSceneDragDropEvent*) override { LOG << "NodeGraphicsItem::dropEvent"; }

	virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
	static int padding();

protected:
	static qreal minHeight();
	static qreal minWidth();

private:
	QRectF outlineRect() const;
	void trackNodes();

private:
	virtual QString getHeaderText() const = 0;
	virtual QString getContentText() const = 0;
	virtual QBrush getHeaderBrush() const = 0;
	virtual QBrush getContentBrush() const;
	virtual NodeGraphicsItem* clone() const = 0;

protected:
	Properties m_properties;

private:
	QList<ArrowLineGraphicsItem*> m_incomingLinks;
	QList<ArrowLineGraphicsItem*> m_outcomingLinks;

	qreal m_width;
	qreal m_height;

	bool m_resizing;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(NodeGraphicsItem::Properties)

#endif // NODEGRAPHICSITEM_H
