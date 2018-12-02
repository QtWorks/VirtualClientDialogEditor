#pragma once

#include "core/dialog.h"
#include "dialoggraphicsinfo.h"

#include <QtSql/QSqlDatabase>

class DialogGraphicsInfoStorage
{
public:
	DialogGraphicsInfoStorage(QString dbPath);

	bool open();

	struct Key
	{
		QString clientId;
		QString dialogName;
		Core::Dialog::Difficulty dialogType;
	};

	QList<PhaseGraphicsInfo> read(const Key& key, const QList<PhaseGraphicsInfo>& defaultValue = {});
	void insert(const Key& key, const QList<PhaseGraphicsInfo>& phasesGraphicsInfo);
	void update(const Key& key, const QList<PhaseGraphicsInfo>& phasesGraphicsInfo);
	void remove(const Key& key);

private:
	QString m_dbPath;
	QSqlDatabase m_database;
};

typedef std::shared_ptr<DialogGraphicsInfoStorage> DialogGraphicsInfoStoragePtr;
