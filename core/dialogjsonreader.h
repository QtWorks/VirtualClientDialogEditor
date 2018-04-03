#pragma once

#include "dialog.h"

namespace Core
{

class DialogJsonReader
{
public:
	DialogJsonReader();

	Dialog read(const QByteArray& json, bool& ok);
};

}
