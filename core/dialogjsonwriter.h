#pragma once

#include "dialog.h"

namespace Core
{

class DialogJsonWriter
{
public:
	DialogJsonWriter();

	QByteArray write(const Dialog& dialog);
};

}
