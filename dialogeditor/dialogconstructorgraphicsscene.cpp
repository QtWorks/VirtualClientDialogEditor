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

void DialogConstructorGraphicsScene::setDefaults(const Core::ErrorReplica& errorReplica, const Optional<QString>& phaseRepeatReplica)
{
	m_phase.setErrorReplica(errorReplica);
	m_phase.repeatReplica() = phaseRepeatReplica;
}

void DialogConstructorGraphicsScene::addConstructorItems()
{
	NodeGraphicsItem* phaseNode = new PhaseGraphicsItem(&m_phase, &m_dialog, NodeGraphicsItem::Draggable, this);
	phaseNode->setPos(QPoint(0, 0));
	addItem(phaseNode);

	NodeGraphicsItem* clientReplicaNode = new ClientReplicaNodeGraphicsItem(&m_replica, NodeGraphicsItem::Draggable, this);
	clientReplicaNode->setPos(QPoint(0, 90));
	addItem(clientReplicaNode);

	NodeGraphicsItem* allowedExpectedWordsNode = new ExpectedWordsNodeGraphicsItem(&m_allowedExpectedWords, NodeGraphicsItem::Draggable, this);
	allowedExpectedWordsNode->setPos(QPoint(0, 180));
	addItem(allowedExpectedWordsNode);

	NodeGraphicsItem* forbiddenExpectedWordsNode = new ExpectedWordsNodeGraphicsItem(&m_forbiddenExpectedWords, NodeGraphicsItem::Draggable, this);
	forbiddenExpectedWordsNode->setPos(QPoint(0, 270));
	addItem(forbiddenExpectedWordsNode);

	const qreal arrowLength = forbiddenExpectedWordsNode->sceneBoundingRect().width() / 2;
	ArrowLineGraphicsItem* link = new ArrowLineGraphicsItem(QPointF(0, 0), QPointF(arrowLength, 0));
	link->setDraggable(true);
	link->setPos(QPoint(0, 370));
	addItem(link);
}
