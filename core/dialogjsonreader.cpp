#include "dialogjsonreader.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace Core
{

namespace
{

ClientReplicaNode* parseClientReplicaNode(const QJsonObject& object)
{
	const QString replica = object["replica"].toString();

	return new ClientReplicaNode(replica);
}

ExpectedWordsNode* parseExpectedWordsNode(const QJsonObject& object)
{
	QList<ExpectedWords> expectedWords;
	const QJsonArray expectedWordsArray = object["expectedWords"].toArray();
	for (const QJsonValue& expectedWord : expectedWordsArray)
	{
		const QJsonObject expectedWordObject = expectedWord.toObject();
		const QString words = expectedWordObject["words"].toString();
		const double score = expectedWordObject["score"].toDouble();

		expectedWords << ExpectedWords(words, score);
	}

	// TODO: how to fill hint in dialog object
	const QString hint = object["hint"].toString();

	return new ExpectedWordsNode(expectedWords, hint);
}

AbstractDialogNode* parseNode(const QJsonObject& object)
{
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
		return result;
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
	const QString name = object["name"].toString();
	const double score = object["score"].toDouble();
	const QJsonObject rootObject = object["root"].toObject();

	return PhaseNode(name, score, parseNode(rootObject));
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

}
