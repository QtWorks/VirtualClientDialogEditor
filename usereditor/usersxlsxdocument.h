#pragma once

#include "core/user.h"
#include <QString>
#include <xlsxdocument.h>

class UsersXlsxDocument
{
public:
	explicit UsersXlsxDocument(QString fileName);

	QList<Core::User> read(bool& ok);
	void write(QList<Core::User> users, bool& ok);

private:
	QXlsx::Document m_document;
};
