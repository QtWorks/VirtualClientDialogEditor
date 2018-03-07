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

	void addNodeToScene(NodeGraphicsItem* node, const QPointF& position);
	void addLineToScene(ArrowLineGraphicsItem* line);

signals:
	void nodeAdded(NodeGraphicsItem* node);
	void nodeRemoved(NodeGraphicsItem* node);

	void nodeSelectionChanged(NodeGraphicsItem* node, bool value);

	void linkAdded(ArrowLineGraphicsItem* link);
	void linkRemoved(ArrowLineGraphicsItem* link);

private:
	// Just to remove red cursor
	virtual void dragMoveEvent(QGraphicsSceneDragDropEvent*) override { }

	virtual void dropEvent(QGraphicsSceneDragDropEvent* event) override;

private:
	void refreshScene();

	void removeNodeFromScene(NodeGraphicsItem* node);
	void removeLinkFromScene(ArrowLineGraphicsItem* line);

private:
	IDialogModel* m_model;
};

#endif // DIALOGVIEW_H
