#include "dialogjsonreader.h"
#include "logger.h"

#include <QVector>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace Core
{

namespace
{

typedef QVector<QPair<QString, QJsonValue::Type>> PropertiesList;

bool hasProperty(const QJsonObject& object, const QString& name, QJsonValue::Type type)
{
	return object.contains(name) && object[name].type() == type;
}

bool hasProperties(const QJsonObject& object, const PropertiesList& properties)
{
	for (const auto& property : properties)
	{
		if (!hasProperty(object, property.first, property.second))
		{
			return false;
		}
	}

	return true;
}

void checkProperties(const QJsonObject& object, const PropertiesList& properties)
{
	if (!hasProperties(object, properties))
	{
		QStringList propertiesList;
		for (const auto& property : properties)
		{
			propertiesList.append(property.first + " (" + QString::number(property.second) + ")");
		}

		std::string message = "Properties not found (or types are incorrect): " + propertiesList.join("; ").toStdString();

		throw std::logic_error(message);
	}
}

ClientReplicaNode* parseClientReplicaNode(const QJsonObject& object)
{
	checkProperties(object, { { "replica", QJsonValue::String } });
	const QString replica = object["replica"].toString();

	return new ClientReplicaNode(replica);
}

ExpectedWordsNode* parseExpectedWordsNode(const QJsonObject& object)
{
	checkProperties(object, {
		{ "expectedWords", QJsonValue::Array },
		{ "forbidden", QJsonValue::Bool }
	});

	const QJsonArray expectedWordsArray = object["expectedWords"].toArray();

	QList<ExpectedWords> expectedWords;
	for (const QJsonValue& expectedWord : expectedWordsArray)
	{
		const QJsonObject expectedWordObject = expectedWord.toObject();
		checkProperties(expectedWordObject, { { "words", QJsonValue::String }, { "score", QJsonValue::Double } });

		expectedWords << ExpectedWords(expectedWordObject["words"].toString(), expectedWordObject["score"].toDouble());
	}

	int minScore = 0;
	if (object.contains("minScore"))
	{
		checkProperties(object, { { "minScore", QJsonValue::Double } });
		minScore = object["minScore"].toDouble();
	}

	bool forbidden = object["forbidden"].toBool();

	if (!object.contains("hint"))
	{
		return new ExpectedWordsNode(expectedWords, minScore, forbidden);
	}

	checkProperties(object, { { "hint", QJsonValue::String } });
	return new ExpectedWordsNode(expectedWords, minScore, object["hint"].toString(), forbidden);
}

AbstractDialogNode* parseNode(const QJsonObject& object)
{
	static const PropertiesList s_requiredProperties = {
		{ "type", QJsonValue::Double },
		{ "data", QJsonValue::Object },
		{ "id", QJsonValue::String },
		{ "childNodes", QJsonValue::Array },
		{ "parentNodes", QJsonValue::Array }
	};
	checkProperties(object, s_requiredProperties);

	const int type = object["type"].toInt();
	const QJsonObject data = object["data"].toObject();

	AbstractDialogNode* result = nullptr;
	if (type == 0)
	{
		result = parseClientReplicaNode(data);
	}
	else if (type == 1)
	{
		result = parseExpectedWordsNode(data);
	}

	if (!result)
	{
		return nullptr;
	}

	const AbstractDialogNode::Id id = object["id"].toString();
	result->setId(id);

	const QJsonArray childsArray = object["childNodes"].toArray();
	for (const QJsonValue& child : childsArray)
	{
		result->appendChild(child.toString());
	}

	const QJsonArray parentsArray = object["parentNodes"].toArray();
	for (const QJsonValue& parent : parentsArray)
	{
		result->appendParent(parent.toString());
	}

	return result;
}

QList<AbstractDialogNode*> parseNodes(const QJsonArray& nodes)
{
	QList<AbstractDialogNode*> result;

	for (const auto& nodeValue : nodes)
	{
		if (!nodeValue.isObject())
		{
			continue;
		}

		const auto nodeObject = nodeValue.toObject();

		AbstractDialogNode* node = parseNode(nodeObject);
		if (node)
		{
			result.append(node);
		}
	}

	return result;
}

ErrorReplica parseError(const QJsonObject& object)
{
	ErrorReplica result;

	if (hasProperty(object, "errorReplica", QJsonValue::String))
	{
		result.errorReplica = object["errorReplica"].toString();
	}

	if (hasProperty(object, "errorPenalty", QJsonValue::Double))
	{
		result.errorPenalty = object["errorPenalty"].toDouble();
	}

	if (hasProperty(object, "finishingExpectedWords", QJsonValue::Array))
	{
		QList<QString> list;

		for (const QJsonValue& expectedWords : object["finishingExpectedWords"].toArray())
		{
			if (expectedWords.isString())
			{
				list.append(expectedWords.toString());
			}
		}

		result.finishingExpectedWords = list;
	}

	if (hasProperty(object, "finishingReplica", QJsonValue::String))
	{
		result.finishingReplica = object["finishingReplica"].toString();
	}

	return result;
}

PhaseNode parsePhase(const QJsonObject& object)
{
	static const PropertiesList s_requiredProperties = {
		{ "id", QJsonValue::String },
		{ "name", QJsonValue::String },
		{ "score", QJsonValue::Double },
		{ "repeatOnInsufficientScore", QJsonValue::Bool },
		{ "nodes", QJsonValue::Array }
	};
	checkProperties(object, s_requiredProperties);

	const QString id = object["id"].toString();
	const QString name = object["name"].toString();
	const double score = object["score"].toDouble();
	const bool repeatOnInsufficientScore = object["repeatOnInsufficientScore"].toBool();
	const QList<AbstractDialogNode*> nodes = parseNodes(object["nodes"].toArray());

	const ErrorReplica errorReplica = object.contains("errorReplica") ? parseError(object["errorReplica"].toObject()) : ErrorReplica();	
	PhaseNode result = PhaseNode(name, score, repeatOnInsufficientScore, nodes, errorReplica);
	result.setId(id);

	if (hasProperty(object, "repeatReplica", QJsonValue::String))
	{
		result.repeatReplica() = object["repeatReplica"].toString();
	}

	return result;
}

}

DialogJsonReader::DialogJsonReader()
{
}

Dialog DialogJsonReader::read(const QByteArray& json, bool& ok)
{
	QJsonParseError error;
	QJsonDocument document = QJsonDocument::fromJson(json, &error);
	if (document.isNull() || !document.isObject())
	{
		ok = false;
		return Dialog();
	}

	const QJsonObject dialogObject = document.object();

	try
	{
		static const PropertiesList s_requiredProperties = {
			{ "name", QJsonValue::String },
			{ "difficulty", QJsonValue::Double },
			{ "note", QJsonValue::String },
			{ "successRatio", QJsonValue::Double },
			{ "phases", QJsonValue::Array },
			{ "groups", QJsonValue::Array }
		};
		checkProperties(dialogObject, s_requiredProperties);

		const QString name = dialogObject["name"].toString();
		const Dialog::Difficulty difficulty = static_cast<Dialog::Difficulty>(dialogObject["difficulty"].toInt());
		const QString note = dialogObject["note"].toString();
		const double successRatio = dialogObject["successRatio"].toDouble() * 100;
		const ErrorReplica errorReplica = parseError(dialogObject["errorReplica"].toObject());

		const QJsonArray phasesArray = dialogObject["phases"].toArray();
		QList<PhaseNode> phases;
		for (const QJsonValue& phase : phasesArray)
		{
			phases << parsePhase(phase.toObject());
		}

		const QJsonArray groupesArray = dialogObject["groups"].toArray();
		QList<QString> groups;
		for (const QJsonValue& group : groupesArray)
		{
			groups << group.toString();
		}

		/*const auto hasField = [&errorReplica, &phases](ErrorReplica::Field field)
		{
			return errorReplica.has(field) ||
				std::all_of(phases.begin(), phases.end(),
					[&field](const PhaseNode& phase) { return phase.errorReplica().has(field); });
		};

		if (!(hasField(ErrorReplica::Field::ErrorReplica) &&
			  hasField(ErrorReplica::Field::ErrorPenalty) &&
			  hasField(ErrorReplica::Field::FinishingExpectedWords) &&
			  hasField(ErrorReplica::Field::FinishingReplica)))
		{
			LOG << "Some error replica fields are missing";
			ok = false;
			return Dialog();
		}*/

		Dialog dialog = Dialog(name, difficulty, note, phases, errorReplica, successRatio, groups);
		if (hasProperty(dialogObject, "phaseRepeatReplica", QJsonValue::String))
		{
			dialog.phaseRepeatReplica = dialogObject["phaseRepeatReplica"].toString();
		}

		ok = true;
		return dialog;
	}
	catch (const std::logic_error& exception)
	{
		LOG << exception.what();
		ok = false;
		return Dialog();
	}
}

}
