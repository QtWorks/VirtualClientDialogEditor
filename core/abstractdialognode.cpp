#include "abstractdialognode.h"

namespace Core
{

AbstractDialogNode::~AbstractDialogNode()
{
}

AbstractDialogNode& AbstractDialogNode::appendParent(AbstractDialogNode* parent)
{
	m_parentNodes.insert(parent);
	return *this;
}

AbstractDialogNode& AbstractDialogNode::removeParent(AbstractDialogNode* parent)
{
	Q_ASSERT(m_parentNodes.contains(parent));
	m_parentNodes.remove(parent);
	return *this;
}

AbstractDialogNode& AbstractDialogNode::appendChild(AbstractDialogNode* child)
{
	m_childNodes.insert(child);
	return *this;
}

AbstractDialogNode& AbstractDialogNode::removeChild(AbstractDialogNode* child)
{
	Q_ASSERT(m_childNodes.contains(child));
	m_childNodes.remove(child);
	return *this;
}

QSet<AbstractDialogNode*> AbstractDialogNode::parentNodes() const
{
	return m_parentNodes;
}

QSet<AbstractDialogNode*> AbstractDialogNode::childNodes() const
{
	return m_childNodes;
}

AbstractDialogNode* AbstractDialogNode::deepCopy() const
{
	AbstractDialogNode* copy = shallowCopy();

	for (const AbstractDialogNode* child : m_childNodes)
	{
		copy->appendChild(child->deepCopy());
	}

	return copy;
}

bool AbstractDialogNode::validate() const
{
	QString error;
	return validate(error);
}

bool AbstractDialogNode::equalTo(AbstractDialogNode* other) const
{
	if (m_childNodes.size() != other->m_childNodes.size())
	{
		return false;
	}

	return std::all_of(m_childNodes.begin(), m_childNodes.end(),
		[&other](AbstractDialogNode* child)
		{
			return std::any_of(other->m_childNodes.begin(), other->m_childNodes.end(),
				[&child](AbstractDialogNode* anotherChild) { return child->equalTo(anotherChild); });
		}
	);
}

}
