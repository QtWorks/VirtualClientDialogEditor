#pragma once

#include "dialog.h"

namespace Core
{

class DialogJsonWriter
{
public:
	DialogJsonWriter();

	QString write(const Dialog& dialog);
};

}
