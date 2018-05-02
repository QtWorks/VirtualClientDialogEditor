#include "dialogjsonwriter.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace Core
{

namespace
{

QJsonObject dumpClientReplicaNode(const ClientReplicaNode& node)
{
	return {
		{ "replica", node.replica }
	};
}

QJsonObject dumpExpectedWordsNode(const ExpectedWordsNode& node)
{
	QJsonArray expectedWords;
	for (const ExpectedWords& words : node.expectedWords)
	{
		expectedWords.append(QJsonObject({
			{ "words", words.words },
			{ "score", words.score }
		}));
	}

	QJsonObject result = {
		{ "expectedWords", expectedWords }
	};

	if (node.customHint)
	{
		result.insert("hint", node.hint);
	}

	return result;
}

QJsonObject dumpNode(const AbstractDialogNode* node)
{
	QJsonObject result;

	if (dynamic_cast<const ClientReplicaNode*>(node))
	{
		result["type"] = 0;
		result["data"] = dumpClientReplicaNode(dynamic_cast<const ClientReplicaNode&>(*node));
	}
	else if (dynamic_cast<const ExpectedWordsNode*>(node))
	{
		result["type"] = 1;
		result["data"] = dumpExpectedWordsNode(dynamic_cast<const ExpectedWordsNode&>(*node));
	}

	QJsonArray childNodes;
	for (const AbstractDialogNode* child : node->childNodes())
	{
		childNodes.append(dumpNode(child));
	}
	result["childNodes"] = childNodes;

	return result;
}

QJsonArray dumpPhases(const QList<Core::PhaseNode>& phases)
{
	QJsonArray array;

	for (const Core::PhaseNode& phase : phases)
	{
		array.append(QJsonObject({
			{ "name", phase.name },
			{ "score", phase.score },
			{ "root", dumpNode(phase.root) }
		}));
	}

	return array;
}

}

DialogJsonWriter::DialogJsonWriter()
{
}

QString DialogJsonWriter::write(const Dialog& dialog, bool compact)
{
	const QJsonObject object = writeToObject(dialog);
	const QJsonDocument document = QJsonDocument(object);
	return QString::fromLatin1(document.toJson((compact ? QJsonDocument::Compact : QJsonDocument::Indented)));
}

QJsonObject DialogJsonWriter::writeToObject(const Dialog& dialog)
{
	return QJsonObject{
		{ "name", dialog.name },
		{ "difficulty", static_cast<int>(dialog.difficulty) },
		{ "phases", dumpPhases(dialog.phases) }
	};
}

}
