#ifndef ARROWLINEGRAPHICSITEM_H
#define ARROWLINEGRAPHICSITEM_H

#include <QGraphicsLineItem>

class NodeGraphicsItem;

class ArrowLineGraphicsItem
	: public QObject
	, public QGraphicsLineItem
{
	Q_OBJECT

public:
	enum
	{
		Type = UserType + 1
	};

	ArrowLineGraphicsItem(const QPointF& startPoint, const QPointF& endPoint, bool isDraggable, QGraphicsItem* parent = 0);
	ArrowLineGraphicsItem(NodeGraphicsItem* root, NodeGraphicsItem* child, bool isDraggable, QGraphicsItem* parent = 0);
	~ArrowLineGraphicsItem();

	virtual int type() const override;
	QRectF boundingRect() const override;
	QPainterPath shape() const override;

	void updatePosition();
	void updatePosition(const QPointF& p1, const QPointF& p2, bool updateItems = true);
	void setDraggable(bool value);

signals:
	void removeRequested();

protected:
	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) override;

	virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;

private:
	class Item
	{
	public:
		explicit Item(const QPointF& point);
		Item(NodeGraphicsItem* item);
		Item(NodeGraphicsItem* item, ArrowLineGraphicsItem* lineItem, bool incomingLink);
		Item(const Item& item);
		Item(Item&& item);
		~Item();

		Item& operator=(const Item& item);
		Item& operator=(Item&& item);

		void addIncomingLink(ArrowLineGraphicsItem* link);
		void addOutcomingLink(ArrowLineGraphicsItem* link);
		void removeIncomingLink(ArrowLineGraphicsItem* link);
		void removeOutcomingLink(ArrowLineGraphicsItem* link);

		void swap(Item& item);

		bool collidesWithItem(const Item& item) const;
		bool canChangeTo(const Item& item) const;
		QPointF intersectionPoint(const QLineF& line) const;

		QPointF point() const;

		friend class ArrowLineGraphicsItem;

	private:
		enum class Type
		{
			Point,
			GraphicsItem
		};

		Type m_type;
		QPointF m_point;

		ArrowLineGraphicsItem* m_lineItem;
		bool m_isIncomingLink;
		NodeGraphicsItem* m_item;
	};

private:
	ArrowLineGraphicsItem(const Item& startItem, const Item& endItem, bool isDraggable, QGraphicsItem* parent);

	void updatePosition(Item&& item);
	void updateArrowHead();
	QList<QGraphicsItem*> nodesAtPosition(const QPointF& position) const;
	// itemAtPosition && canMoveTo must be const, beat those compile errors!
	Item itemAtPosition(const QPointF& position, const bool link);
	bool canMoveTo(const QPointF& position);

	static int anchorRadius();
	static bool atResizeEllipse(const QPointF& ellipseCenter, const QPointF& point);

	enum class MoveMode
	{
		NoMove,
		MoveStartPoint,
		MoveEndPoint,
		MoveWholeArrow
	};

	bool m_isDraggable;
	MoveMode m_moveMode;
	Item m_startItem;
	Item m_endItem;
	QPolygonF m_arrowHead;

	QLineF m_line;
};

#endif // ARROWLINEGRAPHICSITEM_H
