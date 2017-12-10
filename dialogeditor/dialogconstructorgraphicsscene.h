#ifndef DIALOGCONSTRUCTORGRAPHICSSCENE_H
#define DIALOGCONSTRUCTORGRAPHICSSCENE_H

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
};

#endif // DIALOGCONSTRUCTORGRAPHICSSCENE_H
