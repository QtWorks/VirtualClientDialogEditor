#ifndef DIALOGVIEW_H
#define DIALOGVIEW_H

#include "core/idialogmodel.h"
#include <QGraphicsView>
#include <QGraphicsScene>

#include <QMap>

class NodeGraphicsItem;
class ArrowLineGraphicsItem;

class DialogGraphicsScene
	: public QGraphicsScene
{
	Q_OBJECT

public:
	DialogGraphicsScene(QObject* parent = nullptr);
	~DialogGraphicsScene();

	void setModel(IDialogModel* model);

signals:
	void nodeAdded();
	void nodeRemoved();

	void nodesConnected();

	void linkAdded();
	void linkRemoved();

private:
	// Just to remove red cursor
	virtual void dragMoveEvent(QGraphicsSceneDragDropEvent*) override { }

	virtual void dropEvent(QGraphicsSceneDragDropEvent* event) override;

private:
	void refreshScene();

	void addNodeToScene(NodeGraphicsItem* node, const QPointF& position);
	//void addLineToScene(NodeGraphicsItem* parent, NodeGraphicsItem* child);
	void addLineToScene(ArrowLineGraphicsItem* line);
	void removeNodeFromScene(NodeGraphicsItem* node);
	void removeLinkFromScene(ArrowLineGraphicsItem* line);

private:
	IDialogModel* m_model;
};

#endif // DIALOGVIEW_H
