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

bool hasProperties(const QJsonObject& object, const PropertiesList& properties)
{
	for (const auto& property : properties)
	{
		if (!object.contains(property.first) || object[property.first].type() != property.second)
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
	QList<ExpectedWords> expectedWords;

	checkProperties(object, { { "expectedWords", QJsonValue::Array } });
	const QJsonArray expectedWordsArray = object["expectedWords"].toArray();

	for (const QJsonValue& expectedWord : expectedWordsArray)
	{
		const QJsonObject expectedWordObject = expectedWord.toObject();
		checkProperties(expectedWordObject, { { "words", QJsonValue::String }, { "score", QJsonValue::Double } });

		expectedWords << ExpectedWords(expectedWordObject["words"].toString(), expectedWordObject["score"].toDouble());
	}

	if (!object.contains("hint"))
	{
		return new ExpectedWordsNode(expectedWords);
	}

	checkProperties(object, { { "hint", QJsonValue::String } });
	return new ExpectedWordsNode(expectedWords, object["hint"].toString());
}

AbstractDialogNode* parseNode(const QJsonObject& object)
{
	static const PropertiesList s_requiredProperties = {
		{ "type", QJsonValue::Double },
		{ "data", QJsonValue::Object },
		{ "childNodes", QJsonValue::Array }
	};
	checkProperties(object, s_requiredProperties);

	const int type = object["type"].toInt();
	const QJsonObject data = object["data"].toObject();

	AbstractDialogNode* result = nullptr;

	switch (type)
	{
	case 0: result = parseClientReplicaNode(data); break;
	case 1: result = parseExpectedWordsNode(data); break;
	}

	if (!result)
	{
		return nullptr;
	}

	const QJsonArray childsArray = object["childNodes"].toArray();
	for (const QJsonValue& child : childsArray)
	{
		result->appendChild(parseNode(child.toObject()));
	}

	return result;
}

PhaseNode parsePhase(const QJsonObject& object)
{
	static const PropertiesList s_requiredProperties = {
		{ "name", QJsonValue::String },
		{ "score", QJsonValue::Double },
		{ "root", QJsonValue::Object }
	};
	checkProperties(object, s_requiredProperties);

	const QString name = object["name"].toString();
	const double score = object["score"].toDouble();
	const QJsonObject rootObject = object["root"].toObject();

	std::unique_ptr<AbstractDialogNode> rootNode(parseNode(rootObject));
	return PhaseNode(name, score, rootNode.get());
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
			{ "phases", QJsonValue::Array }
		};
		checkProperties(dialogObject, s_requiredProperties);

		const QString name = dialogObject["name"].toString();
		const Dialog::Difficulty difficulty = static_cast<Dialog::Difficulty>(dialogObject["difficulty"].toInt());

		const QJsonArray phasesArray = dialogObject["phases"].toArray();
		QList<PhaseNode> phases;
		for (const QJsonValue& phase : phasesArray)
		{
			phases << parsePhase(phase.toObject());
		}

		ok = true;
		return Dialog(name, difficulty, phases);
	}
	catch (const std::logic_error& exception)
	{
		LOG << exception.what();
		ok = false;
		return Dialog();
	}
}

}
