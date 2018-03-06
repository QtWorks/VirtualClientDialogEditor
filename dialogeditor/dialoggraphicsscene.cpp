#include "dialoggraphicsscene.h"
#include "clientreplicanodegraphicsitem.h"
#include "expectedwordsnodegraphicsitem.h"
#include "arrowlinegraphicsitem.h"
#include "nodegraphicsitemmimedata.h"
#include "arrowlinegraphicsitemmimedata.h"
#include "logger.h"

#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>

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

		//LOG << "ArrowLineGraphicsItem " << ARG(event->scenePos()) << ARG(item->line());
	}
	else if (const auto* mimeData = qobject_cast<const NodeGraphicsItemMimeData*>(event->mimeData()))
	{
		NodeGraphicsItem* item = mimeData->item();
		item->setParent(this);

		addNodeToScene(item, event->scenePos());

		item->showNodeEditor();

		//LOG << "NodeGraphicsItem -> " << sceneRect();
	}
	else
	{
		QGraphicsScene::dropEvent(event);
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
	int replicaIndex = 0;

	for (int i = 0; i < phases.size(); ++i)
	{
		const QList<Replica>& replicas = phases[i].replicas;
		for (int j = 0; j < replicas.size(); ++j)
		{
			const Replica& replica = replicas[j];

			NodeGraphicsItem* clientReplicaNode = new ClientReplicaNodeGraphicsItem(replica.clientReplica,
				NodeGraphicsItem::Resizable | NodeGraphicsItem::Editable | NodeGraphicsItem::Removable, this);
			addNodeToScene(clientReplicaNode, QPointF(0, 100 * replicaIndex++));

			if (lastInsertedNode != nullptr)
			{
				addLineToScene(new ArrowLineGraphicsItem(lastInsertedNode, clientReplicaNode, false));
			}

			if (!replica.expectedWords.isEmpty())
			{
				NodeGraphicsItem* expectedWordsNode = new ExpectedWordsNodeGraphicsItem(replica.expectedWords,
					NodeGraphicsItem::Resizable | NodeGraphicsItem::Editable | NodeGraphicsItem::Removable, this);
				addNodeToScene(expectedWordsNode, QPointF(0, 100 * replicaIndex++));

				addLineToScene(new ArrowLineGraphicsItem(clientReplicaNode, expectedWordsNode, false));

				lastInsertedNode = expectedWordsNode;
			}
		}
	}
}

void DialogGraphicsScene::addNodeToScene(NodeGraphicsItem* node, const QPointF& position)
{
	node->setPos(position);

	QObject::connect(node, &NodeGraphicsItem::removeRequested, [this, node]()
	{
		removeNodeFromScene(node);
	});

	addItem(node);

	emit nodeAdded();
}

void DialogGraphicsScene::addLineToScene(ArrowLineGraphicsItem* line)
{
	QObject::connect(line, &ArrowLineGraphicsItem::removeRequested, [this, line]()
	{
		removeLinkFromScene(line);
	});

	addItem(line);

	emit linkAdded();
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

	emit nodeRemoved();
}

void DialogGraphicsScene::removeLinkFromScene(ArrowLineGraphicsItem* link)
{
	if (link->scene() == nullptr)
	{
		return;
	}

	removeItem(link);

	emit linkRemoved();
}
