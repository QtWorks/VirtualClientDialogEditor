#ifndef NODEGRAPHICSITEM_H
#define NODEGRAPHICSITEM_H

#include "core/dialog.h"
#include "logger.h"
#include <QGraphicsObject>
#include <QBrush>

class ArrowLineGraphicsItem;
class PhaseGraphicsItem;

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
	void resize(qreal width, qreal height);
	void resizeToMinimal();

	QPointF position() const;
	void setPosition(const QPointF& position);

	void addIncomingLink(ArrowLineGraphicsItem* link);
	void removeIncomingLink(ArrowLineGraphicsItem* link);
	QList<ArrowLineGraphicsItem*> incomingLinks() const;

	void addOutcomingLink(ArrowLineGraphicsItem* link);
	void removeOutcomingLink(ArrowLineGraphicsItem* link);
	QList<ArrowLineGraphicsItem*> outcomingLinks() const;

	PhaseGraphicsItem* getPhase() const;
	void setPhase(PhaseGraphicsItem* phase);

	static bool atResizeRect(const QPointF& position, const QRectF& rect);

	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/ = Q_NULLPTR) override;
	virtual QRectF boundingRect() const override;
	virtual void showNodeEditor() = 0;

	virtual Core::AbstractDialogNode* data() = 0;
	virtual const Core::AbstractDialogNode* data() const = 0;

	virtual qreal minHeight() const;
	virtual qreal minWidth() const;

signals:
	void removeRequested();
	void selectionChanged(bool value);
	void positionChanged(QPointF oldPosition, QPointF newPosition);
	void changed(NodeGraphicsItem* newNode);

public:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
	virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;

	virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
	static int padding();

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
	QPointF m_position;

	bool m_resizing;

	PhaseGraphicsItem* m_phase;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(NodeGraphicsItem::Properties)

#endif // NODEGRAPHICSITEM_H
