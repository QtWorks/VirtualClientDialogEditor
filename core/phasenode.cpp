#include "phasenode.h"
#include "expectedwordsnode.h"

#include "logger.h"

namespace Core
{

namespace
{

QList<AbstractDialogNode*> filterLeafs(const QList<AbstractDialogNode*>& nodes)
{
	QList<AbstractDialogNode*> leafs;
	std::copy_if(nodes.begin(), nodes.end(), std::back_inserter(leafs),
		[&nodes](AbstractDialogNode* node)
		{
			const QSet<AbstractDialogNode::Id> nodeChilds = node->childNodes();
			if (nodeChilds.empty())
			{
				return true;
			}

			return std::any_of(nodeChilds.begin(), nodeChilds.end(), [&nodes](AbstractDialogNode::Id childId)
			{
				const auto childNodeIt = findNodeById(nodes.begin(), nodes.end(), childId);
				return childNodeIt == nodes.end();
			});
		});
	return leafs;
}

typedef QList<AbstractDialogNode*> NodesPath;

QList<NodesPath> findPathsToRoot(AbstractDialogNode* node, const QList<AbstractDialogNode*>& nodes)
{
	LOG << "searching path to root from " << node->id();

	if (node->parentNodes().empty())
	{
		LOG << node->id() << " is root";
		return { { node } };
	}

	QList<NodesPath> path;

	for (const AbstractDialogNode::Id& parentId : node->parentNodes())
	{
		const auto parentNodeIt = findNodeById(nodes.begin(), nodes.end(), parentId);
		if (parentNodeIt == nodes.end())
		{
			LOG << node->id() << " is root";
			return { { node } };
		}

		QList<NodesPath> pathToParent = findPathsToRoot(*parentNodeIt, nodes);
		for (NodesPath& path : pathToParent)
		{
			path.prepend(node);
		}

		LOG << "found " << pathToParent.length() << " paths to " << node->id();

		path.append(pathToParent);
	}

	return path;
}

double calculateBestPossibleScore(const QString& name, const QList<AbstractDialogNode*>& nodes)
{
	QList<AbstractDialogNode*> leafs = filterLeafs(nodes);

	typedef QList<AbstractDialogNode*> NodesPath;
	QList<NodesPath> leafToRootPaths;
	for (AbstractDialogNode* leaf : leafs)
	{
		QList<NodesPath> paths = findPathsToRoot(leaf, nodes);

		leafToRootPaths.append(paths);
	}

	LOG << name << ": Found " << leafToRootPaths.length() << " leafToRoot paths";

	QList<double> scores;
	std::transform(leafToRootPaths.begin(), leafToRootPaths.end(), std::back_inserter(scores),
		[](const NodesPath& path)
		{
			double score = 0.0;
			for (AbstractDialogNode* node : path)
			{
				if (node->type() != ExpectedWordsNode::Type)
				{
					continue;
				}

				const QList<ExpectedWords>& expectedWords = node->as<ExpectedWordsNode>()->expectedWords();
				if (expectedWords.empty())
				{
					continue;
				}

				const auto maxScoreIt = std::max_element(expectedWords.begin(), expectedWords.end(),
					[](const ExpectedWords& left, const ExpectedWords& right)
					{
						return left.score < right.score;
					});
				Q_ASSERT(maxScoreIt != expectedWords.end());
				score += (*maxScoreIt).score;
			}
			return score;
		});

	LOG << name << ": Possible scores: " << scores;

	const auto maxScoreIt = std::max_element(scores.begin(), scores.end());
	Q_ASSERT(maxScoreIt != scores.end());
	return *maxScoreIt;
}

}

PhaseNode::PhaseNode(const QString& name, double score, const QList<AbstractDialogNode*>& nodes, const ErrorReplica& errorReplica)
	: m_name(name)
	, m_score(score)
	, m_nodes(nodes)
	, m_errorReplica(errorReplica)
{
}

PhaseNode::PhaseNode(const PhaseNode& other)
	: m_name(other.m_name)
	, m_score(other.m_score)
	, m_errorReplica(other.m_errorReplica)
	, m_repeatReplica(other.m_repeatReplica)
{
	setId(other.id());

	for (const AbstractDialogNode::Id& id : other.parentNodes())
	{
		appendParent(id);
	}

	for (const AbstractDialogNode::Id& id : other.childNodes())
	{
		appendChild(id);
	}

	for (AbstractDialogNode* node : other.nodes())
	{
		m_nodes.append(node->clone(false));
	}
}

const QString& PhaseNode::name() const
{
	return m_name;
}

void PhaseNode::setName(const QString& name)
{
	m_name = name;
}

double PhaseNode::score() const
{
	return m_score;
}

void PhaseNode::setScore(double score)
{
	m_score = score;
}

const QList<AbstractDialogNode*>& PhaseNode::nodes() const
{
	return m_nodes;
}

void PhaseNode::appendNode(AbstractDialogNode* node)
{
	if (!m_nodes.contains(node))
	{
		m_nodes.append(node);
	}
}

void PhaseNode::removeNode(AbstractDialogNode* node)
{
	m_nodes.removeOne(node);
}

const ErrorReplica& PhaseNode::errorReplica() const
{
	return m_errorReplica;
}

ErrorReplica& PhaseNode::errorReplica()
{
	return m_errorReplica;
}

void PhaseNode::setErrorReplica(const ErrorReplica& replica)
{
	m_errorReplica = replica;
}

void PhaseNode::resetErrorReplica()
{
	m_errorReplica = ErrorReplica();
}

bool PhaseNode::hasRepeatReplica() const
{
	return m_repeatReplica.isValid();
}

QString PhaseNode::repeatReplica() const
{
	return m_repeatReplica.toString();
}

void PhaseNode::setRepeatReplica(const QString& value)
{
	m_repeatReplica = value;
}

void PhaseNode::resetRepeatReplica()
{
	m_repeatReplica = QVariant();
}

int PhaseNode::type() const
{
	return PhaseNode::Type;
}

bool PhaseNode::validate(QString& errorMessage) const
{
	if (m_name.trimmed().isEmpty())
	{
		errorMessage = "Имя фазы не может быть пустым";
		return false;
	}

	if (!m_nodes.empty())
	{
		const double bestPossibleScore = calculateBestPossibleScore(m_name, m_nodes);
		if (bestPossibleScore < m_score)
		{
			errorMessage = "Cлишком большое количество баллов (максимум - " + QString::number(bestPossibleScore) + ")";
			return false;
		}
	}

	if (m_errorReplica.hasErrorReplica() && m_errorReplica.errorReplica().trimmed().isEmpty())
	{
		errorMessage = "Реплика для ошибки не может быть пустой";
		return false;
	}

	if (m_errorReplica.hasErrorPenalty() && m_errorReplica.errorPenalty() <= 0.0)
	{
		errorMessage = "Количество штрафных баллов должно быть больше 0";
		return false;
	}

	if (m_errorReplica.hasFinishingReplica() && m_errorReplica.finishingReplica().trimmed().isEmpty())
	{
		errorMessage = "Завершающая реплика не может быть пустой";
		return false;
	}

	if (m_errorReplica.hasFinishingExpectedWords() && m_errorReplica.finishingExpectedWords().isEmpty())
	{
		errorMessage = "Завершающие опорные слова не могут быть пустыми";
		return false;
	}

	if (hasRepeatReplica() && repeatReplica().trimmed().isEmpty())
	{
		errorMessage = "Реплика для повтора не может быть пустой";
		return false;
	}

	return true;
}

AbstractDialogNode* PhaseNode::shallowCopy() const
{
	QList<AbstractDialogNode*> clonedNodes;
	for (AbstractDialogNode* node : m_nodes)
	{
		clonedNodes.append(node->clone(false));
	}

	return new PhaseNode(m_name, m_score, clonedNodes, m_errorReplica);
}

bool PhaseNode::compareData(AbstractDialogNode* other) const
{
	Q_ASSERT(other->type() == type());
	return *this == *other->as<PhaseNode>();
}

bool operator==(const PhaseNode& left, const PhaseNode& right)
{
	return left.name() == right.name() &&
		left.score() == right.score() &&
		left.errorReplica() == right.errorReplica() &&
		left.nodes().size() == right.nodes().size() &&
		std::equal(left.nodes().begin(), left.nodes().end(), right.nodes().begin(),
			[](AbstractDialogNode* left, AbstractDialogNode* right) { return left->compare(right); });
}

}
