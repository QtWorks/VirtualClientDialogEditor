#include "usersxlsxdocument.h"

UsersXlsxDocument::UsersXlsxDocument(QString fileName)
	: m_document(fileName)
{
}

QList<Core::User> UsersXlsxDocument::read(bool& ok)
{
	QXlsx::CellRange cellRange = m_document.dimension();
	if (cellRange.columnCount() < 3)
	{
		ok = false;
		return {};
	}

	const int usernameColumn = cellRange.firstColumn();
	const int passwordColumn = cellRange.firstColumn() + 1;
	const int groupsColumn = cellRange.firstColumn() + 2;

	QList<Core::User> result;

	for (int row = cellRange.firstRow() + 1; row <= cellRange.lastRow(); ++row)
	{
		Core::User user;

		QVariant usernameVariant = m_document.read(row, usernameColumn);
		user.name = usernameVariant.isNull() ? "" : usernameVariant.toString().trimmed();

		QVariant passwordVariant = m_document.read(row, passwordColumn);
		user.password = passwordVariant.isNull() ? "" : passwordVariant.toString().trimmed();

		QVariant groupsVariant = m_document.read(row, groupsColumn);
		if (!groupsVariant.isNull())
		{
			QList<QString> groups = groupsVariant.toString().trimmed().split(",", QString::SkipEmptyParts);
			for (QString group : groups)
			{
				group = group.trimmed();
				if (!group.isEmpty())
				{
					user.groups << group;
				}
			}
		}

		result << user;
	}

	ok = true;
	return result;
}

void UsersXlsxDocument::write(QList<User> users, bool& ok)
{
	m_document.write("A1", "Имя пользователя");
	m_document.write("B1", "Доступные группы");
	m_document.write("C1", "Заблокированные группы");

	for (int i = 0; i < users.size(); ++i)
	{
		const int row = i + 2;
		m_document.write("A" + QString::number(row), users[i].name);
		m_document.write("B" + QString::number(row), users[i].availableGroups.join(", "));
		m_document.write("C" + QString::number(row), users[i].bannedGroups.join(", "));
	}

	ok = m_document.save();
}



