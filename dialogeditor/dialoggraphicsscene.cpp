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

// TODO: expectedwords can have only 1 outgoing link

namespace
{

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

}

DialogGraphicsScene::DialogGraphicsScene(QObject* parent)
	: QGraphicsScene(parent)
{
}

DialogGraphicsScene::~DialogGraphicsScene()
{
}

void DialogGraphicsScene::setModel(IDialogModel* model)
{
	m_model = model;

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

		const QRectF itemSceneRect = QRectF(event->scenePos().x(), event->scenePos().y(), item->boundingRect().width(), item->boundingRect().height());
		const QList<PhaseGraphicsItem*> phases = phaseItems(itemSceneRect);

		if (item->type() == ClientReplicaNodeGraphicsItem::Type ||
			item->type() == ExpectedWordsNodeGraphicsItem::Type)
		{
			if (phases.size() == 1)
			{
				PhaseGraphicsItem* phase = phases.first();
				if (!phase->sceneBoundingRect().contains(itemSceneRect))
				{
					return;
				}

				phase->addItem(item);
			}
			else if (!phases.empty())
			{
				return;
			}
		}
		else if (item->type() == PhaseGraphicsItem::Type)
		{
			if (!phases.empty())
			{
				return;
			}
		}

		// TODO: sometimes item not shown on scene
		item->setParent(this);

		addNodeToScene(item, event->scenePos());

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
	const QList<Phase>& phases = m_model->phases();
	if (phases.empty())
	{
		return;
	}

	NodeGraphicsItem* lastInsertedNode = nullptr;
	PhaseGraphicsItem* lastInsertedPhase = nullptr;

	// TODO: move paddings into PhaseGraphicsItem object?
	static const qreal s_nodesInterval = 30.0;
	static const qreal s_phaseTopPadding = 45.0;
	static const qreal s_phaseRightPadding = 15.0;
	static const qreal s_phaseBottomPadding = 15.0;
	static const qreal s_phaseLeftPadding = 15.0;

	const NodeGraphicsItem::Properties nodeProperties = NodeGraphicsItem::Resizable | NodeGraphicsItem::Editable | NodeGraphicsItem::Removable;
	for (int phaseIndex = 0; phaseIndex < phases.size(); ++phaseIndex)
	{
		const Phase& phase = phases[phaseIndex];

		PhaseGraphicsItem* phaseItem = new PhaseGraphicsItem(phase, nodeProperties, this);
		QPointF phasePos = QPointF(0.0, lastInsertedPhase ? lastInsertedPhase->sceneBoundingRect().bottom() + s_nodesInterval : 0.0);
		addNodeToScene(phaseItem, phasePos);

		const QRectF phaseRect = phaseItem->sceneBoundingRect();

		NodeGraphicsItem* firstNode = nullptr;

		for (int replicaIndex = 0; replicaIndex < phase.replicas.size(); ++replicaIndex)
		{
			const Replica& replica = phase.replicas[replicaIndex];

			NodeGraphicsItem* clientReplicaNode = new ClientReplicaNodeGraphicsItem(replica.clientReplica, nodeProperties, this);
			if (!firstNode)
			{
				addNodeToScene(clientReplicaNode, QPointF(phaseRect.left() + s_phaseLeftPadding, phaseRect.top() + s_phaseTopPadding));
				firstNode = clientReplicaNode;
			}
			else
			{
				addNodeToScene(clientReplicaNode, QPointF(phaseRect.left() + s_phaseLeftPadding, lastInsertedNode->sceneBoundingRect().bottom() + s_nodesInterval));
			}
			phaseItem->addItem(clientReplicaNode);

			if (lastInsertedNode != nullptr)
			{
				addLineToScene(new ArrowLineGraphicsItem(lastInsertedNode, clientReplicaNode, false));
			}

			if (!replica.expectedWords.isEmpty())
			{
				NodeGraphicsItem* expectedWordsNode = new ExpectedWordsNodeGraphicsItem(replica.expectedWords, nodeProperties, this);
				addNodeToScene(expectedWordsNode, QPointF(phaseRect.left() + s_phaseLeftPadding, clientReplicaNode->sceneBoundingRect().bottom() + s_nodesInterval));
				phaseItem->addItem(expectedWordsNode);

				addLineToScene(new ArrowLineGraphicsItem(clientReplicaNode, expectedWordsNode, false));

				lastInsertedNode = expectedWordsNode;
			}
		}

		phaseItem->resize(
			lastInsertedNode->sceneBoundingRect().width() + s_phaseLeftPadding + s_phaseRightPadding,
			(lastInsertedNode->sceneBoundingRect().bottom() - phaseItem->sceneBoundingRect().top()) + s_phaseBottomPadding);

		lastInsertedPhase = phaseItem;
	}
}

void DialogGraphicsScene::addNodeToScene(NodeGraphicsItem* node, const QPointF& position)
{
	node->setPosition(position);

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

	addItem(node);

	emit nodeAdded(node);
}

void DialogGraphicsScene::addLineToScene(ArrowLineGraphicsItem* line)
{
	QObject::connect(line, &ArrowLineGraphicsItem::removeRequested, [this, line]()
	{
		removeLinkFromScene(line);
	});

	addItem(line);

	emit linkAdded(line);
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

	emit nodeRemoved(node);
}

void DialogGraphicsScene::removeLinkFromScene(ArrowLineGraphicsItem* link)
{
	if (link->scene() == nullptr)
	{
		return;
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
	phaseItem->setPosition(from);
}

void DialogGraphicsScene::onReplicaPositionChanged(NodeGraphicsItem* replicaItem, const QPointF& from, const QPointF& to)
{
	const QRectF oldRect = QRectF(from.x(), from.y(), replicaItem->boundingRect().width(), replicaItem->boundingRect().height());
	const QRectF newRect = QRectF(to.x(), to.y(), replicaItem->boundingRect().width(), replicaItem->boundingRect().height());

	LOG << ARG(oldRect) << ARG(newRect);

	const QList<PhaseGraphicsItem*> phasesAtOldRect = phaseItems(oldRect);
	const QList<PhaseGraphicsItem*> phasesAtNewRect = phaseItems(newRect);
	// TODO: assert that both collections has no more that 1 element

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
		}
		else if (intersects(phaseItem, replicaItem))
		{
			LOG << "phase.intersects -> revert";
			replicaItem->setPosition(from);
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
		}

		return;
	}

	if (phasesAtOldRect != phasesAtNewRect)
	{
		LOG << "different phases -> revert";
		replicaItem->setPosition(from);
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
		replicaItem->setPosition(from);
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
