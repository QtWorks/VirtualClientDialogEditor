#ifndef DIALOGVIEW_H
#define DIALOGVIEW_H

#include "nodegraphicsitem.h"
#include "graphlayout.h"
#include <QGraphicsView>
#include <QGraphicsScene>

#include <QMap>

class ArrowLineGraphicsItem;
class PhaseGraphicsItem;

class DialogGraphicsScene
	: public QGraphicsScene
{
	Q_OBJECT

public:
	DialogGraphicsScene(QObject* parent = nullptr);
	~DialogGraphicsScene();

	void setDialog(Core::Dialog* dialog);

	void addNodeToScene(NodeGraphicsItem* node, const QPointF& position);
	void addLineToScene(ArrowLineGraphicsItem* line);
	void connectNodes(NodeGraphicsItem* parentNode, NodeGraphicsItem* childNode,
		const QVector<QPointF>& intermediatePoints = {});

signals:
	void nodeSelectionChanged(NodeGraphicsItem* node, bool value);

	void linkAdded(ArrowLineGraphicsItem* link);
	void linkRemoved(ArrowLineGraphicsItem* link);

	void nodeAdded(NodeGraphicsItem* node);
	void nodeRemoved(NodeGraphicsItem* node);
	void nodeChanged(NodeGraphicsItem* node);

	void nodesConnected(NodeGraphicsItem* parent, NodeGraphicsItem* child);
	void nodesDisconnected(NodeGraphicsItem* parent, NodeGraphicsItem* child);

	void nodeAddedToPhase(NodeGraphicsItem* node, PhaseGraphicsItem* phase);
	void nodeRemovedFromPhase(NodeGraphicsItem* node, PhaseGraphicsItem* phase);

private:
	// Just to remove red cursor
	virtual void dragMoveEvent(QGraphicsSceneDragDropEvent*) override { }

	virtual void dropEvent(QGraphicsSceneDragDropEvent* event) override;
	virtual void keyPressEvent(QKeyEvent *event) override;

private:
	void refreshScene();

	typedef std::map<QString, NodeGraphicsItem*> NodeItemById;

	std::pair<PhaseGraphicsItem*, NodeItemById> renderPhase(Core::PhaseNode& phase, int phaseIndex);
	NodeItemById renderNodes(PhaseGraphicsItem* phaseItem, const GraphLayout::NodesByLayer& nodes, const QList<Core::AbstractDialogNode*>& dataNodes);
	void renderEdges(PhaseGraphicsItem* phaseItem, const GraphLayout::NodesByLayer& nodes, const NodeItemById& itemByNode);

	void removeNodeFromScene(NodeGraphicsItem* node);
	void removeLinkFromScene(ArrowLineGraphicsItem* line);

	void onPhasePositionChanged(PhaseGraphicsItem* phaseItem, const QPointF& from, const QPointF& to);
	void onReplicaPositionChanged(NodeGraphicsItem* replicaItem, const QPointF& from, const QPointF& to);
	QList<PhaseGraphicsItem*> phaseItems(const QRectF& rect) const;

private:
	Core::Dialog* m_dialog;
};

#endif // DIALOGVIEW_H
