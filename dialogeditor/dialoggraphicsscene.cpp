#include "dialoggraphicsscene.h"
#include "clientreplicanodegraphicsitem.h"
#include "expectedwordsnodegraphicsitem.h"
#include "arrowlinegraphicsitem.h"
#include "nodegraphicsitemmimedata.h"
#include "arrowlinegraphicsitemmimedata.h"
#include "phasegraphicsitem.h"
#include "logger.h"

#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QKeyEvent>

namespace
{

static const qreal s_nodesInterval = 30.0;
static const qreal s_phaseTopPadding = 45.0;
static const qreal s_phaseRightPadding = 15.0;
static const qreal s_phaseBottomPadding = 15.0;
static const qreal s_phaseLeftPadding = 15.0;

bool contains(PhaseGraphicsItem* phase, NodeGraphicsItem* node)
{
	Q_ASSERT(node->type() != PhaseGraphicsItem::Type);
	return phase->sceneBoundingRect().contains(node->sceneBoundingRect());
}

bool intersects(PhaseGraphicsItem* phase, NodeGraphicsItem* node)
{
	Q_ASSERT(node->type() != PhaseGraphicsItem::Type);
	return phase->sceneBoundingRect().intersects(node->sceneBoundingRect());
}

NodeGraphicsItem* makeGraphicsItem(Core::AbstractDialogNode* node, NodeGraphicsItem::Properties properties, QGraphicsScene* parent)
{
	if (node->type() == Core::ClientReplicaNode::Type)
	{
		return new ClientReplicaNodeGraphicsItem(node->as<Core::ClientReplicaNode>(), properties, parent);
	}

	if (node->type() == Core::ExpectedWordsNode::Type)
	{
		return new ExpectedWordsNodeGraphicsItem(node->as<Core::ExpectedWordsNode>(), properties, parent);
	}

	return nullptr;
}

GraphLayout::GraphNodeData makeNode(Core::AbstractDialogNode::Id label, int layer)
{
	GraphLayout::GraphNodeData node;
	node.label = label;
	node.layer = layer;
	return node;
}

int calcLayer(const Core::AbstractDialogNode* node, const QList<Core::AbstractDialogNode*>& nodes)
{
	if (node->parentNodes().empty())
	{
		return 0;
	}

	std::vector<int> parentLayers;
	for (const auto& parentNodeId : node->parentNodes())
	{
		const auto parentNodeIt = Core::findNodeById(nodes, parentNodeId);
		if (parentNodeIt == nodes.end())
		{
			continue;
		}

		const int layer = calcLayer(*parentNodeIt, nodes);

		parentLayers.push_back(layer);
	}

	const auto maxLayerIt = std::max_element(parentLayers.begin(), parentLayers.end());
	return maxLayerIt == parentLayers.end() ? 0 : (*maxLayerIt + 1);
}

GraphLayout::GraphData makeGraphData(QList<Core::AbstractDialogNode*> nodes)
{
	std::vector<GraphLayout::GraphNodeData> nodeList;
	for (const auto& node : nodes)
	{
		nodeList.push_back(makeNode(node->id(), calcLayer(node, nodes)));
	}

	std::vector<std::vector<int>> adjList;
	for (const auto& node : nodes)
	{
		std::vector<int> adjacentNodes;

		for (const auto& childId : node->childNodes())
		{
			const auto childNodeIt = Core::findNodeById(nodes, childId);
			if (childNodeIt == nodes.end())
			{
				continue;
			}

			const int nodeIndex = std::distance(nodes.begin(), childNodeIt);

			adjacentNodes.push_back(nodeIndex);
		}

		adjList.push_back(adjacentNodes);
	}

	const auto maxLayerIt = std::max_element(nodeList.begin(), nodeList.end(),
		[](const GraphLayout::GraphNodeData& left, const GraphLayout::GraphNodeData& right) { return left.layer < right.layer; });
	const int totalLayers = maxLayerIt->layer + 1;

	return { nodeList, adjList, totalLayers };
}

}

DialogGraphicsScene::DialogGraphicsScene(QObject* parent)
	: QGraphicsScene(parent)
{
	LOG;
}

DialogGraphicsScene::~DialogGraphicsScene()
{
	LOG;
}

void DialogGraphicsScene::setDialog(Core::Dialog* dialog, QList<PhaseGraphicsInfo> phasesGraphicsInfo)
{
	m_dialog = dialog;
	m_phasesGraphicsInfo = phasesGraphicsInfo;

	refreshScene();
}

void DialogGraphicsScene::dropEvent(QGraphicsSceneDragDropEvent* event)
{
	if (const auto* mimeData = qobject_cast<const ArrowLineGraphicsItemMimeData*>(event->mimeData()))
	{
		ArrowLineGraphicsItem* item = mimeData->item();
		item->setParent(this);

		const QPointF lineStart = event->scenePos();
		const QPointF lineEnd = QPointF(lineStart.x() + item->line().length(), lineStart.y());
		item->updatePosition(lineStart, lineEnd);

		addLineToScene(item);
	}
	else if (const auto* mimeData = qobject_cast<const NodeGraphicsItemMimeData*>(event->mimeData()))
	{
		NodeGraphicsItem* item = mimeData->item();
		item->resizeToMinimal();

		const QPointF dropPosition = event->scenePos();
		const QRectF itemBoundingRect = item->boundingRect();
		const QRectF itemSceneRect = QRectF(dropPosition.x(), dropPosition.y(), itemBoundingRect.width(), itemBoundingRect.height());

		const QList<PhaseGraphicsItem*> phases = phaseItems(itemSceneRect);

		if (item->type() == ClientReplicaNodeGraphicsItem::Type ||
			item->type() == ExpectedWordsNodeGraphicsItem::Type)
		{
			if (phases.size() == 1)
			{
				PhaseGraphicsItem* phase = phases.first();
				if (!phase->sceneBoundingRect().contains(itemSceneRect))
				{
					LOG << "Phase SBR not contains " << itemSceneRect;
					return;
				}

				phase->addItem(item);
				emit nodeAddedToPhase(item, phase);
			}
			else if (!phases.empty())
			{
				LOG << "No phases found at " << itemSceneRect;
				return;
			}
		}
		else if (item->type() == PhaseGraphicsItem::Type)
		{
			qgraphicsitem_cast<PhaseGraphicsItem*>(item)->setDialog(m_dialog);

			if (!phases.empty())
			{
				return;
			}
		}

		item->setParent(this);

		addNodeToScene(item, dropPosition);
		LOG << "Drop item at " << dropPosition << ARG2(item->pos(), "itemPosition");

		item->showNodeEditor();
	}
	else
	{
		QGraphicsScene::dropEvent(event);
	}
}

void DialogGraphicsScene::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Delete)
	{
		const QList<QGraphicsItem*> items = selectedItems();
		for (QGraphicsItem* item : items)
		{
			if (item->type() == ArrowLineGraphicsItem::Type)
			{
				qgraphicsitem_cast<ArrowLineGraphicsItem*>(item)->keyPressEvent(event);
			}
			else if (item->type() == ClientReplicaNodeGraphicsItem::Type ||
				item->type() == ExpectedWordsNodeGraphicsItem::Type ||
				item->type() == PhaseGraphicsItem::Type)
			{
				qgraphicsitem_cast<NodeGraphicsItem*>(item)->keyPressEvent(event);
			}
		}
	}
}

void DialogGraphicsScene::refreshScene()
{
	QList<Core::PhaseNode>& phases = m_dialog->phases;
	if (phases.empty())
	{
		return;
	}

	std::vector<PhaseGraphicsItem*> phaseItems;
	NodeItemById graphicsItemByLabel;
	std::vector<std::pair<Core::AbstractDialogNode::Id, Core::AbstractDialogNode::Id>> nodesBetweenPhases;

	// place phases
	for (int phaseIndex = 0; phaseIndex < phases.size(); ++phaseIndex)
	{
		Core::PhaseNode& phase = phases[phaseIndex];

		auto phaseGraphicsInfoIt = std::find_if(m_phasesGraphicsInfo.begin(), m_phasesGraphicsInfo.end(),
			[&](const PhaseGraphicsInfo& info)
			{
				return info.name == phase.name();
			});
		PhaseGraphicsInfo phaseGraphicsInfo = phaseGraphicsInfoIt == m_phasesGraphicsInfo.end() ? PhaseGraphicsInfo() : *phaseGraphicsInfoIt;

		const std::pair<PhaseGraphicsItem*, NodeItemById> phaseInfo = renderPhase(phase, phaseIndex, phaseGraphicsInfo);
		phaseItems.push_back(phaseInfo.first);
		graphicsItemByLabel.insert(phaseInfo.second.begin(), phaseInfo.second.end());

		for (const std::pair<QString, NodeGraphicsItem*> nodeByLabel : phaseInfo.second)
		{
			NodeGraphicsItem* node = nodeByLabel.second;

			for (const Core::AbstractDialogNode::Id childNodeId : node->data()->childNodes())
			{
				const auto childIt = std::find_if(phaseInfo.second.begin(), phaseInfo.second.end(),
					[&childNodeId](const std::pair<QString, NodeGraphicsItem*>& x)
					{
						return x.second->data()->id() == childNodeId;
					});

				if (childIt == phaseInfo.second.end())
				{
					nodesBetweenPhases.push_back({ node->data()->id(), childNodeId });
				}
			}
		}
	}

	// connect phases
	for (const auto& nodesPair : nodesBetweenPhases)
	{
		NodeGraphicsItem* from = graphicsItemByLabel[nodesPair.first];
		NodeGraphicsItem* to = graphicsItemByLabel[nodesPair.second];

		connectNodes(from, to);
	}
}

std::pair<PhaseGraphicsItem*, DialogGraphicsScene::NodeItemById> DialogGraphicsScene::renderPhase(
	Core::PhaseNode& phase, int phaseIndex, const PhaseGraphicsInfo& phaseGraphicsInfo)
{
	const NodeGraphicsItem::Properties nodeProperties = NodeGraphicsItem::Resizable | NodeGraphicsItem::Editable | NodeGraphicsItem::Removable;

	PhaseGraphicsItem* phaseItem = new PhaseGraphicsItem(&phase, m_dialog, nodeProperties, this);

	ClientReplicaNodeGraphicsItem dummy(nullptr, NodeGraphicsItem::NoProperties);

	QPointF phasePos;

	if (phaseGraphicsInfo.position.isNull())
	{
		phasePos = QPointF((dummy.minWidth() + s_phaseLeftPadding + s_phaseRightPadding + s_nodesInterval) * phaseIndex, 0.0);
	}
	else
	{
		phasePos = phaseGraphicsInfo.position;
	}

	LOG << "Place phase #" << phaseIndex << " at " << phasePos;
	addNodeToScene(phaseItem, phasePos);

	const auto phaseGraph = makeGraphData(phase.nodes());
	GraphLayout layout(phaseGraph.totalLayers);
	const GraphLayout::NodesByLayer graph = layout.render(phaseGraph);

	const auto itemByNode = renderNodes(phaseItem, graph, phase.nodes());
	placeNodes(phaseItem, itemByNode, graph, phaseGraphicsInfo.nodes);

	renderEdges(phaseItem, graph, itemByNode);

	if (phaseGraphicsInfo.size.isValid())
	{
		phaseItem->resize(phaseGraphicsInfo.size.width(), phaseGraphicsInfo.size.height());
	}
	else
	{
		phaseItem->resizeToMinimal();
	}

	return { phaseItem, itemByNode };
}

QPointF nodePosition(const PhaseGraphicsItem& phase, const GraphLayout::GraphNode& node)
{
	const QPointF phasePosition = phase.scenePos();

	const int x = phasePosition.x() + s_phaseLeftPadding + node.x * 210;
	const int y = phasePosition.y() + s_phaseTopPadding + node.y * (60 + s_nodesInterval);

	return QPointF(x, y);
}

DialogGraphicsScene::NodeItemById DialogGraphicsScene::renderNodes(PhaseGraphicsItem* phaseItem,
	const GraphLayout::NodesByLayer& nodes, const QList<Core::AbstractDialogNode*>& dataNodes)
{
	NodeItemById itemByNode;

	for (const auto& nodesByLayer : nodes)
	{
		for (const auto& graphNode : nodesByLayer.second)
		{
			if (graphNode.virt)
			{
				continue;
			}

			const NodeGraphicsItem::Properties nodeProperties = NodeGraphicsItem::Resizable | NodeGraphicsItem::Editable | NodeGraphicsItem::Removable;

			const auto nodeIt = Core::findNodeById(dataNodes, graphNode.label);
			NodeGraphicsItem* nodeGraphicsItem = makeGraphicsItem(*nodeIt, nodeProperties, this);

			emit nodeAddedToPhase(nodeGraphicsItem, phaseItem);

			phaseItem->addItem(nodeGraphicsItem);

			itemByNode.insert({ graphNode.label, nodeGraphicsItem });
		}
	}

	return itemByNode;
}

template <typename Key, typename Value>
QList<Value> values(const std::map<Key, Value>& m)
{
	QList<Value> result;

	for (const auto& x : m)
	{
		result.append(x.second);
	}

	return result;
}

void DialogGraphicsScene::placeNodes(
	PhaseGraphicsItem* phaseItem, const NodeItemById& nodeItemById, const GraphLayout::NodesByLayer& graph, const QList<NodeGraphicsInfo>& nodesGraphicsInfo)
{
	QList<NodeGraphicsItem*> nodes = values(nodeItemById);

	auto newItemIt = std::partition(nodes.begin(), nodes.end(),
		[&](NodeGraphicsItem* node)
		{
			const auto& id = node->data()->id();
			return std::find_if(nodesGraphicsInfo.begin(), nodesGraphicsInfo.end(),
				[&](const NodeGraphicsInfo& x) { return x.id == id; }) != nodesGraphicsInfo.end();
		});

	if (nodesGraphicsInfo.isEmpty() || newItemIt == nodes.begin())
	{
		for (const auto& nodesByLayer : graph)
		{
			for (const auto& graphNode : nodesByLayer.second)
			{
				if (graphNode.virt)
				{
					continue;
				}

				auto nodeIt = nodeItemById.find(graphNode.label);
				if (nodeIt == nodeItemById.end())
				{
					continue;
				}

				NodeGraphicsItem* nodeGraphicsItem = nodeIt->second;
				const QPointF position = nodePosition(*phaseItem, graphNode);
				addNodeToScene(nodeGraphicsItem, position);
			}
		}

		return;
	}

	for (auto it = nodes.begin(); it != newItemIt; ++it)
	{
		NodeGraphicsItem* nodeGraphicsItem = *it;

		auto nodeGraphicsInfoIt = std::find_if(nodesGraphicsInfo.begin(), nodesGraphicsInfo.end(),
			[&](const NodeGraphicsInfo& info){ return info.id == nodeGraphicsItem->data()->id(); });
		Q_ASSERT(nodeGraphicsInfoIt != nodesGraphicsInfo.end());
		const NodeGraphicsInfo& nodeGraphicsInfo = *nodeGraphicsInfoIt;

		addNodeToScene(nodeGraphicsItem, nodeGraphicsInfo.position);

		if (nodeGraphicsInfo.size.isValid())
		{
			nodeGraphicsItem->resize(nodeGraphicsInfo.size.width(), nodeGraphicsInfo.size.height());
		}
	}

	for (auto it = newItemIt; it != nodes.end(); ++it)
	{
		GraphLayout::GraphNode node;
		node.x = 0;
		node.y = 0;

		QPointF position = nodePosition(*phaseItem, node);
		addNodeToScene(*it, position);
	}
}

void DialogGraphicsScene::renderEdges(PhaseGraphicsItem* phaseItem, const GraphLayout::NodesByLayer& nodes, const NodeItemById& itemByNode)
{
	for (const auto& nodesByLayer : nodes)
	{
		for (const GraphLayout::GraphNode& node : nodesByLayer.second)
		{
			if (node.virt)
			{
				continue;
			}

			NodeGraphicsItem* root = itemByNode.at(node.label);

			for (const auto& targetNode : node.trgNodes)
			{
				const int x = targetNode[0];
				const int y = targetNode[1];
				const GraphLayout::GraphNode* adjNode = &(nodes.at(y)[x]);

				QVector<QPointF> intermediatePoints;

				while (adjNode->virt)
				{
					// Not the best way, but...
					ClientReplicaNodeGraphicsItem dummy(nullptr, NodeGraphicsItem::NoProperties);
					QPointF position = nodePosition(*phaseItem, *adjNode) + QPointF(dummy.minWidth() / 2, dummy.minHeight() / 2);
					intermediatePoints.push_back(position);

					Q_ASSERT(adjNode->trgNodes.size() == 1);

					const GraphLayout::Point& targetNode = adjNode->trgNodes[0];
					const int x = targetNode[0];
					const int y = targetNode[1];
					adjNode = &(nodes.at(y)[x]);
				}

				NodeGraphicsItem* child = itemByNode.at(adjNode->label);
				connectNodes(root, child, intermediatePoints);
				LOG << "Connect nodes "
					<< root->data()->id() << " (" << root->pos() << ")"
					<< " and " << child->data()->id() << " (" << child->pos() << ")"
					<< " via intermediate points " << intermediatePoints;
			}
		}
	}
}

void DialogGraphicsScene::addNodeToScene(NodeGraphicsItem* node, const QPointF& position)
{
	node->setPos(position);

	connect(node, &NodeGraphicsItem::removeRequested, [this, node]()
	{
		removeNodeFromScene(node);
	});

	connect(node, &NodeGraphicsItem::selectionChanged, [this, node](bool value)
	{
		emit nodeSelectionChanged(node, value);
	});

	connect(node, &NodeGraphicsItem::positionChanged, [this, node](QPointF oldPosition, QPointF newPosition)
	{
		if (node->type() == PhaseGraphicsItem::Type)
		{
			onPhasePositionChanged(qgraphicsitem_cast<PhaseGraphicsItem*>(node), oldPosition, newPosition);
		}
		else
		{
			onReplicaPositionChanged(node, oldPosition, newPosition);
		}
	});

	connect(node, &NodeGraphicsItem::changed, [this, node]()
	{
		emit nodeChanged(node);
	});

	addItem(node);

	emit nodeAdded(node);
}

void DialogGraphicsScene::addLineToScene(ArrowLineGraphicsItem* line)
{
	QObject::connect(line, &ArrowLineGraphicsItem::removeRequested, [this, line]()
	{
		removeLinkFromScene(line);
	});

	QObject::connect(line, &ArrowLineGraphicsItem::nodesConnected, [this, line](NodeGraphicsItem* parent, NodeGraphicsItem* child)
	{
		emit nodesConnected(parent, child);
	});

	QObject::connect(line, &ArrowLineGraphicsItem::nodesDisconnected, [this, line](NodeGraphicsItem* parent, NodeGraphicsItem* child)
	{
		emit nodesDisconnected(parent, child);
	});

	addItem(line);

	emit linkAdded(line);
}

void DialogGraphicsScene::connectNodes(NodeGraphicsItem* parentNode, NodeGraphicsItem* childNode,
	const QVector<QPointF>& intermediatePoints)
{
	ArrowLineGraphicsItem* line = new ArrowLineGraphicsItem(parentNode, childNode, intermediatePoints);
	line->setDraggable(false);

	addLineToScene(line);

	emit nodesConnected(parentNode, childNode);
}

void DialogGraphicsScene::removeNodeFromScene(NodeGraphicsItem* node)
{
	for (const auto& link : node->incomingLinks())
	{
		removeLinkFromScene(link);
	}

	for (const auto& link : node->outcomingLinks())
	{
		removeLinkFromScene(link);
	}

	removeItem(node);

	if (node->type() != PhaseGraphicsItem::Type && node->getPhase())
	{
		emit nodeRemovedFromPhase(node, node->getPhase());
	}

	emit nodeRemoved(node);
}

void DialogGraphicsScene::removeLinkFromScene(ArrowLineGraphicsItem* link)
{
	if (link->scene() == nullptr)
	{
		return;
	}

	if (link->isConnectingNodes())
	{
		emit nodesDisconnected(link->parentNode(), link->childNode());
	}

	removeItem(link);

	delete link;
	//emit linkRemoved(link);
}

void DialogGraphicsScene::onPhasePositionChanged(PhaseGraphicsItem* phaseItem, const QPointF& from, const QPointF& to)
{
	const QRectF newRect = QRectF(to.x(), to.y(), phaseItem->boundingRect().width(), phaseItem->boundingRect().height());
	const QList<PhaseGraphicsItem*> phasesAtNewRect = phaseItems(newRect);

	if (phasesAtNewRect.empty() ||
		(phasesAtNewRect.size() == 1 && phasesAtNewRect.first() == phaseItem))
	{
		LOG << "Phase moved to " << to;

		return;
	}

	LOG << "Found " << phasesAtNewRect.size() << " phases";
	phaseItem->setPos(from);
}

void DialogGraphicsScene::onReplicaPositionChanged(NodeGraphicsItem* replicaItem, const QPointF& from, const QPointF& to)
{
	const QRectF oldRect = QRectF(from.x(), from.y(), replicaItem->boundingRect().width(), replicaItem->boundingRect().height());
	const QRectF newRect = QRectF(to.x(), to.y(), replicaItem->boundingRect().width(), replicaItem->boundingRect().height());

	LOG << ARG(oldRect) << ARG(newRect);

	const QList<PhaseGraphicsItem*> phasesAtOldRect = phaseItems(oldRect);
	const QList<PhaseGraphicsItem*> phasesAtNewRect = phaseItems(newRect);

	if (phasesAtOldRect.empty() && phasesAtNewRect.empty())
	{
		return;
	}

	if (phasesAtOldRect.empty() && !phasesAtNewRect.empty())
	{
		PhaseGraphicsItem* phaseItem = phasesAtNewRect.first();
		if (contains(phaseItem, replicaItem))
		{
			LOG << "phase.contains -> add";
			phaseItem->addItem(replicaItem);
			emit nodeAddedToPhase(replicaItem, phaseItem);
		}
		else if (intersects(phaseItem, replicaItem))
		{
			LOG << "phase.intersects -> revert";
			replicaItem->setPos(from);
		}

		return;
	}

	if (!phasesAtOldRect.empty() && phasesAtNewRect.empty())
	{
		PhaseGraphicsItem* phaseItem = phasesAtOldRect.first();
		if (!contains(phaseItem, replicaItem))
		{
			LOG << "!phase.contains -> remove";
			phaseItem->removeItem(replicaItem);
			emit nodeRemovedFromPhase(replicaItem, phaseItem);
		}

		return;
	}

	if (phasesAtOldRect != phasesAtNewRect)
	{
		LOG << "different phases -> revert";
		replicaItem->setPos(from);
		return;
	}

	PhaseGraphicsItem* phaseItem = phasesAtNewRect.first();
	if (contains(phaseItem, replicaItem))
	{
		LOG << "phase.contains -> do nothing";
		return;
	}

	if (intersects(phaseItem, replicaItem))
	{
		LOG << "phase.intersects -> revert";
		replicaItem->setPos(from);
	}
}

QList<PhaseGraphicsItem*> DialogGraphicsScene::phaseItems(const QRectF& rect) const
{
	QList<PhaseGraphicsItem*> result;
	for (QGraphicsItem* item : items(rect))
	{
		if (item->type() == PhaseGraphicsItem::Type)
		{
			result.push_back(qgraphicsitem_cast<PhaseGraphicsItem*>(item));
		}
	}
	return result;
}
