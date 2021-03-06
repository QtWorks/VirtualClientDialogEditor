#include "abstractdialognode.h"
#include "hashcombine.h"
#include <QDateTime>

namespace Core
{

AbstractDialogNode::AbstractDialogNode()
	: m_id(QString::number(QDateTime::currentMSecsSinceEpoch()))
{
}

AbstractDialogNode::~AbstractDialogNode()
{
}

AbstractDialogNode::Id AbstractDialogNode::id() const
{
	return m_id;
}

void AbstractDialogNode::setId(Id id)
{
	m_id = id;
}

QSet<AbstractDialogNode::Id> AbstractDialogNode::parentNodes() const
{
	return m_parentNodes;
}

void AbstractDialogNode::appendParent(const Id& id)
{
	m_parentNodes.insert(id);
}

void AbstractDialogNode::removeParent(const Id& id)
{
	Q_ASSERT(m_parentNodes.contains(id));
	m_parentNodes.remove(id);
}

QSet<AbstractDialogNode::Id> AbstractDialogNode::childNodes() const
{
	return m_childNodes;
}

void AbstractDialogNode::appendChild(const Id& id)
{
	m_childNodes.insert(id);
}

void AbstractDialogNode::removeChild(const Id& id)
{
	Q_ASSERT(m_childNodes.contains(id));
	m_childNodes.remove(id);
}

AbstractDialogNode* AbstractDialogNode::clone(bool uniqueId) const
{
	AbstractDialogNode* result = shallowCopy();

	if (!uniqueId)
	{
		result->m_id = m_id;
	}

	result->m_parentNodes = m_parentNodes;
	result->m_childNodes = m_childNodes;

	return result;
}

bool AbstractDialogNode::validate() const
{
	QString error;
	return validate(error);
}

bool AbstractDialogNode::compare(AbstractDialogNode* other) const
{
	const auto rank = [](const AbstractDialogNode* node)
	{
		return std::make_tuple(node->id(), node->type(), node->childNodes(), node->parentNodes());
	};

	return rank(this) == rank(other) && compareData(other);
}

size_t AbstractDialogNode::hash() const
{
	size_t seed = 0;

	for (Id id : m_parentNodes)
	{
		hashCombine(seed, id);
	}

	for (Id id : m_childNodes)
	{
		hashCombine(seed, id);
	}

	hashCombine(seed, m_id);

	seed ^= calculateHash();

	return seed;
}

}
