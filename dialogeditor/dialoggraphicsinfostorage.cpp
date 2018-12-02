#include "dialoggraphicsinfostorage.h"

#include <QPointF>
#include <QSizeF>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <QSqlQuery>

namespace
{

bool hasProperties(QJsonObject object, QList<QPair<QString, QJsonValue::Type>> properties)
{
	for (auto property : properties)
	{
		if (!object.contains(property.first) || object[property.first].type() != property.second)
		{
			return false;
		}
	}
	return true;
}

QList<PhaseGraphicsInfo> fromJson(QByteArray json)
{
	QJsonParseError err;
	QJsonDocument document = QJsonDocument::fromJson(json, &err);
	if (document.isNull() || !document.isArray())
	{
		return {};
	}

	QJsonArray phasesArray = document.array();

	QList<PhaseGraphicsInfo> result;
	for (QJsonValue phaseJsonValue : phasesArray)
	{
		if (!phaseJsonValue.isObject())
		{
			continue;
		}

		QJsonObject phaseJsonObject = phaseJsonValue.toObject();
		if (!hasProperties(phaseJsonObject,
			{
				{ "name", QJsonValue::String },
				{ "position", QJsonValue::Object },
				{ "size", QJsonValue::Object },
				{ "nodes", QJsonValue::Array }
			}))
		{
			continue;
		}

		QJsonObject positionObject = phaseJsonObject["position"].toObject();
		if (!hasProperties(positionObject, { { "x", QJsonValue::Double }, { "y", QJsonValue::Double } }))
		{
			continue;
		}

		QJsonObject sizeObject = phaseJsonObject["size"].toObject();
		if (!hasProperties(sizeObject, { { "w", QJsonValue::Double }, { "h", QJsonValue::Double } }))
		{
			continue;
		}

		PhaseGraphicsInfo phaseGraphicsInfo;
		phaseGraphicsInfo.name = phaseJsonObject["name"].toString();

		phaseGraphicsInfo.position.setX(positionObject["x"].toInt());
		phaseGraphicsInfo.position.setY(positionObject["y"].toInt());

		phaseGraphicsInfo.size.setWidth(sizeObject["w"].toInt());
		phaseGraphicsInfo.size.setHeight(sizeObject["h"].toInt());

		phaseGraphicsInfo.nodes = ([&]() -> QList<NodeGraphicsInfo>
		{
			QList<NodeGraphicsInfo> result;

			QJsonArray nodesArray = phaseJsonObject["nodes"].toArray();

			for (QJsonValue nodeJsonValue : nodesArray)
			{
				if (!nodeJsonValue.isObject())
				{
					continue;
				}

				QJsonObject nodeJsonObject = nodeJsonValue.toObject();
				if (!hasProperties(nodeJsonObject, { { "id", QJsonValue::String }, { "position", QJsonValue::Object }, { "size", QJsonValue::Object } }))
				{
					continue;
				}

				QJsonObject positionObject = nodeJsonObject["position"].toObject();
				if (!hasProperties(positionObject, { { "x", QJsonValue::Double }, { "y", QJsonValue::Double } }))
				{
					continue;
				}

				QJsonObject sizeObject = nodeJsonObject["size"].toObject();
				if (!hasProperties(sizeObject, { { "w", QJsonValue::Double }, { "h", QJsonValue::Double } }))
				{
					continue;
				}

				NodeGraphicsInfo nodeGraphicsInfo;
				nodeGraphicsInfo.id = nodeJsonObject["id"].toString();

				nodeGraphicsInfo.position.setX(positionObject["x"].toInt());
				nodeGraphicsInfo.position.setY(positionObject["y"].toInt());

				nodeGraphicsInfo.size.setWidth(sizeObject["w"].toInt());
				nodeGraphicsInfo.size.setHeight(sizeObject["h"].toInt());

				result.append(nodeGraphicsInfo);
			}

			return result;
		})();
		result.append(phaseGraphicsInfo);
	}
	return result;
}

QString toJson(const QList<PhaseGraphicsInfo>& phasesGraphicsInfo)
{
	QJsonArray result;

	for (const auto& phaseGraphicsInfo : phasesGraphicsInfo)
	{
		QJsonObject phaseJsonObject;
		phaseJsonObject["name"] = phaseGraphicsInfo.name;
		phaseJsonObject["position"] = QJsonObject({ { "x", (int)phaseGraphicsInfo.position.x() }, { "y", (int)phaseGraphicsInfo.position.y() } });
		phaseJsonObject["size"] = QJsonObject({ { "w", (int)phaseGraphicsInfo.size.width() }, { "h", (int)phaseGraphicsInfo.size.height() } });
		phaseJsonObject["nodes"] = ([&]() -> QJsonArray
		{
			QJsonArray nodesArray;
			for (const auto& nodeGraphicsInfo : phaseGraphicsInfo.nodes)
			{
				QJsonObject nodeJsonObject;
				nodeJsonObject["id"] = nodeGraphicsInfo.id;
				nodeJsonObject["position"] = QJsonObject({ { "x", (int)nodeGraphicsInfo.position.x() }, { "y", (int)nodeGraphicsInfo.position.y() } });
				nodeJsonObject["size"] = QJsonObject({ { "w", (int)nodeGraphicsInfo.size.width() }, { "h", (int)nodeGraphicsInfo.size.height() } });
				nodesArray.append(nodeJsonObject);
			}
			return nodesArray;
		})();
		result.append(phaseJsonObject);
	}

	QJsonDocument document = QJsonDocument(result);
	return document.toJson(QJsonDocument::Compact);
}

}

DialogGraphicsInfoStorage::DialogGraphicsInfoStorage(QString dbPath)
	: m_dbPath(dbPath)
{
}

bool DialogGraphicsInfoStorage::open()
{
	m_database = QSqlDatabase::addDatabase("QSQLITE");
	m_database.setDatabaseName(m_dbPath);
	if (!m_database.open())
	{
		return false;
	}

	const QString createTableQuery = "CREATE TABLE dialog_graphics_info ("
		"clientName TEXT, "
		"dialogName TEXT, "
		"dialogType INTEGER, "
		"graphicsInfo TEXT, "
		"PRIMARY KEY (clientName, dialogName, dialogType)"
	")";

	// TODO: logs?

	return QSqlQuery(createTableQuery).exec();
}

QList<PhaseGraphicsInfo> DialogGraphicsInfoStorage::read(const Key& key, const QList<PhaseGraphicsInfo>& defaultValue)
{
	QSqlQuery query;
	query.prepare("SELECT * FROM dialog_graphics_info WHERE clientName = :clientName AND dialogName = (:dialogName) AND dialogType = (:dialogType)");
	query.bindValue(":clientName", key.clientId);
	query.bindValue(":dialogName", key.dialogName);
	query.bindValue(":dialogType", static_cast<int>(key.dialogType));

	if (!query.exec() || !query.next())
	{
		// TODO: logs?
		return defaultValue;
	}

	QByteArray graphicsInfo = query.value("graphicsInfo").toByteArray();
	return fromJson(graphicsInfo);
}

void DialogGraphicsInfoStorage::insert(const Key& key, const QList<PhaseGraphicsInfo>& phasesGraphicsInfo)
{
	QSqlQuery query;
	query.prepare("INSERT INTO dialog_graphics_info (clientName, dialogName, dialogType, graphicsInfo) VALUES (:clientName, :dialogName, :dialogType, :graphicsInfo)");
	query.bindValue(":clientName", key.clientId);
	query.bindValue(":dialogName", key.dialogName);
	query.bindValue(":dialogType", static_cast<int>(key.dialogType));
	query.bindValue(":graphicsInfo", toJson(phasesGraphicsInfo));

	query.exec();

	// TODO: logs?
}

void DialogGraphicsInfoStorage::update(const Key& key, const QList<PhaseGraphicsInfo>& phasesGraphicsInfo)
{
	QSqlQuery query;
	query.prepare("REPLACE INTO dialog_graphics_info (clientName, dialogName, dialogType, graphicsInfo) VALUES (:clientName, :dialogName, :dialogType, :graphicsInfo)");
	query.bindValue(":clientName", key.clientId);
	query.bindValue(":dialogName", key.dialogName);
	query.bindValue(":dialogType", static_cast<int>(key.dialogType));
	query.bindValue(":graphicsInfo", toJson(phasesGraphicsInfo));

	query.exec();

	// TODO: logs?
}

void DialogGraphicsInfoStorage::remove(const Key& key)
{
	QSqlQuery query;
	query.prepare("DELETE FROM dialog_graphics_info WHERE clientName = (:clientName) AND dialogName = (:dialogName) AND dialogType = (:dialogType)");
	query.bindValue(":clientName", key.clientId);
	query.bindValue(":dialogName", key.dialogName);
	query.bindValue(":dialogType", static_cast<int>(key.dialogType));

	query.exec();

	// TODO: logs?
}
