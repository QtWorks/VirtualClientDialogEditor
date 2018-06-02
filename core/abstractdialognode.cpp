#include "abstractdialognode.h"
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

AbstractDialogNode* AbstractDialogNode::clone() const
{
	AbstractDialogNode* result = shallowCopy();

	result->m_id = m_id;
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

}
