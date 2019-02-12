#pragma once

#include "applicationsettings.h"
#include "core/dialog.h"
#include <QGraphicsScene>

class DialogConstructorGraphicsScene
	: public QGraphicsScene
{
public:
	DialogConstructorGraphicsScene(ApplicationSettings* settings, QObject* parent = nullptr);
	~DialogConstructorGraphicsScene();

private:
	// Just to remove red cursor
	virtual void dragMoveEvent(QGraphicsSceneDragDropEvent*) override { }

private:
	void addConstructorItems();

private:
	Core::Dialog m_dialog;
	Core::PhaseNode m_phase { "", 0, false, {}, {} };
	Core::ClientReplicaNode m_replica { "" };
	Core::ExpectedWordsNode m_allowedExpectedWords { {}, false };
	Core::ExpectedWordsNode m_forbiddenExpectedWords { {}, true };
};
