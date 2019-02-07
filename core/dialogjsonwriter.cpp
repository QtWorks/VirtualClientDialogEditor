#include "dialogjsonwriter.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace Core
{

namespace
{

template <typename T>
QJsonArray dump(const T& container,
	std::function<QJsonValue(const typename T::value_type&)> valueConterter = [](const typename T::value_type& value) { return QJsonValue(value); })
{
	QJsonArray array;
	for (const auto& value : container)
	{
		array.append(valueConterter(value));
	}
	return array;
}

QJsonObject dumpClientReplicaNode(const ClientReplicaNode& node)
{
	return {
		{ "replica", node.replica() }
	};
}

QJsonObject dumpExpectedWordsNode(const ExpectedWordsNode& node)
{
	const QJsonArray expectedWords = dump(node.expectedWords(), [](const ExpectedWords& words)
	{
		return QJsonObject({ { "words", words.words }, { "score", words.score } });
	});

	QJsonObject result = {
		{ "expectedWords", expectedWords },
		{ "forbidden", node.forbidden() }
	};

	if (node.customHint())
	{
		result.insert("hint", node.hint());
	}

	return result;
}

QJsonObject dumpNode(const AbstractDialogNode* node)
{
	QJsonObject result;

	result["id"] = node->id();

	if (node->type() == ClientReplicaNode::Type)
	{
		result["type"] = 0;
		result["data"] = dumpClientReplicaNode(dynamic_cast<const ClientReplicaNode&>(*node));
	}
	else if (node->type() == ExpectedWordsNode::Type)
	{
		result["type"] = 1;
		result["data"] = dumpExpectedWordsNode(dynamic_cast<const ExpectedWordsNode&>(*node));
	}

	result["parentNodes"] = dump(node->parentNodes());
	result["childNodes"] = dump(node->childNodes());

	return result;
}

QJsonObject dumpError(const ErrorReplica& error)
{
	QJsonObject result;

	if (error.hasErrorReplica())
	{
		result["errorReplica"] = error.errorReplica();
	}

	if (error.hasErrorPenalty())
	{
		result["errorPenalty"] = error.errorPenalty();
	}

	if (error.hasFinishingExpectedWords())
	{
		result["finishingExpectedWords"] = QJsonArray::fromStringList(error.finishingExpectedWords());
	}

	if (error.hasFinishingReplica())
	{
		result["finishingReplica"] = error.finishingReplica();
	}

	return result;
}

QJsonValue dumpPhase(const Core::PhaseNode& phase)
{
	QJsonObject result = QJsonObject({
		{ "id", phase.id() },
		{ "name", phase.name() },
		{ "score", phase.score() },
		{ "repeatOnInsufficientScore", phase.repeatOnInsufficientScore() },
		{ "nodes", dump(phase.nodes(), dumpNode) }
	});

	if (phase.errorReplica().hasAnyField())
	{
		result["errorReplica"] = dumpError(phase.errorReplica());
	}

	if (phase.hasRepeatReplica())
	{
		result["repeatReplica"] = phase.repeatReplica();
	}

	return result;
}

QJsonValue dumpGroup(const QString& group)
{
	return group;
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
	QJsonObject result = QJsonObject({
		{ "name", dialog.name },
		{ "difficulty", static_cast<int>(dialog.difficulty) },
		{ "note", dialog.note },
		{ "successRatio", (dialog.successRatio / 100) },
		{ "phases", dump(dialog.phases, dumpPhase) },
		{ "groups", dump(dialog.groups, dumpGroup) }
	});

	if (dialog.errorReplica.hasAnyField())
	{
		result["errorReplica"] = dumpError(dialog.errorReplica);
	}

	if (dialog.phaseRepeatReplica.isValid())
	{
		result["phaseRepeatReplica"] = dialog.phaseRepeatReplica.toString();
	}

	return result;
}

}
