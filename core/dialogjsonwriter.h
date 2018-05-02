#pragma once

#include "dialog.h"
#include <QJsonObject>

namespace Core
{

class DialogJsonWriter
{
public:
	DialogJsonWriter();

	QString write(const Dialog& dialog, bool compact = false);
	QJsonObject writeToObject(const Dialog& dialog);
};

}
