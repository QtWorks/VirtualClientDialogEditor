#pragma once

#include "core/dialog.h"
#include <QGraphicsScene>

class DialogConstructorGraphicsScene
	: public QGraphicsScene
{
public:
	DialogConstructorGraphicsScene(QObject* parent = nullptr);
	~DialogConstructorGraphicsScene();

	void setDefaults(const Core::ErrorReplica& errorReplica, const Optional<QString>& phaseRepeatReplica);

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
