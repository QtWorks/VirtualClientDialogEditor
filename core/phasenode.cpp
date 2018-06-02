#include "phasenode.h"

namespace Core
{

PhaseNode::PhaseNode(const QString& name, double score, const QList<AbstractDialogNode*>& nodes)
	: m_name(name)
	, m_score(score)
	, m_nodes(nodes)
{
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

int PhaseNode::type() const
{
	return PhaseNode::Type;
}

bool PhaseNode::validate(QString& error) const
{
	if (m_name.trimmed().isEmpty())
	{
		error = "Имя фазы не может быть пустым";
		return false;
	}

	return true;
}

AbstractDialogNode* PhaseNode::shallowCopy() const
{
	return new PhaseNode(m_name, m_score, m_nodes);
}

bool PhaseNode::compareData(AbstractDialogNode* other) const
{
	Q_ASSERT(other->type() == type());
	return *this == *dynamic_cast<PhaseNode*>(other);
}

bool operator==(const PhaseNode& left, const PhaseNode& right)
{
	return left.name() == right.name() && left.score() == right.score() &&
		left.nodes().size() == right.nodes().size() &&
		std::equal(left.nodes().begin(), left.nodes().end(), right.nodes().begin(),
			[](AbstractDialogNode* left, AbstractDialogNode* right) { return left->compare(right); });
}

}
