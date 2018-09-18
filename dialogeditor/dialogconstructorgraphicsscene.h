#ifndef DIALOGCONSTRUCTORGRAPHICSSCENE_H
#define DIALOGCONSTRUCTORGRAPHICSSCENE_H

#include "core/dialog.h"
#include <QGraphicsScene>


class DialogConstructorGraphicsScene
	: public QGraphicsScene
{
public:
	DialogConstructorGraphicsScene(QObject* parent = nullptr);
	~DialogConstructorGraphicsScene();

private:
	// Just to remove red cursor
	virtual void dragMoveEvent(QGraphicsSceneDragDropEvent*) override { }

private:
	void addConstructorItems();

private:
	Core::Dialog m_dialog;
	Core::PhaseNode m_phase;
	Core::ClientReplicaNode m_replica;
	Core::ExpectedWordsNode m_allowedExpectedWords;
	Core::ExpectedWordsNode m_forbiddenExpectedWords;
};

#endif // DIALOGCONSTRUCTORGRAPHICSSCENE_H
