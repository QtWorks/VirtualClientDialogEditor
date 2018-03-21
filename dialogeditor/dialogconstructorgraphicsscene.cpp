#include "dialogconstructorgraphicsscene.h"
#include "clientreplicanodegraphicsitem.h"
#include "expectedwordsnodegraphicsitem.h"
#include "arrowlinegraphicsitem.h"
#include "phasegraphicsitem.h"

DialogConstructorGraphicsScene::DialogConstructorGraphicsScene(QObject* parent)
	: QGraphicsScene(parent)
{
	addConstructorItems();
}

DialogConstructorGraphicsScene::~DialogConstructorGraphicsScene()
{
}

void DialogConstructorGraphicsScene::addConstructorItems()
{
	NodeGraphicsItem* phaseNode = new PhaseGraphicsItem(Phase(), NodeGraphicsItem::Draggable, this);
	phaseNode->setPos(QPoint(0, 0));
	addItem(phaseNode);

	NodeGraphicsItem* clientReplicaNode = new ClientReplicaNodeGraphicsItem("", NodeGraphicsItem::Draggable, this);
	clientReplicaNode->setPos(QPoint(0, 90));
	addItem(clientReplicaNode);

	NodeGraphicsItem* expectedWordsNode = new ExpectedWordsNodeGraphicsItem({}, NodeGraphicsItem::Draggable, this);
	expectedWordsNode->setPos(QPoint(0, 180));
	addItem(expectedWordsNode);

	const qreal arrowLength = expectedWordsNode->sceneBoundingRect().width() / 2;
	ArrowLineGraphicsItem* link = new ArrowLineGraphicsItem(QPointF(0, 0), QPointF(arrowLength, 0), true);
	link->setPos(QPoint(0, 280));
	addItem(link);
}
