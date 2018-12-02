#pragma once

#include "core/abstractdialognode.h"

#include <QString>
#include <QPoint>
#include <QSize>

struct NodeGraphicsInfo
{
	Core::AbstractDialogNode::Id id;
	QPointF position;
	QSizeF size;
};

struct PhaseGraphicsInfo
{
	QString name;
	QPointF position;
	QSizeF size;
	QList<NodeGraphicsInfo> nodes;
};
