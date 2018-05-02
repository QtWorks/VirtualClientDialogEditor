#include "phasenode.h"

namespace Core
{

PhaseNode::PhaseNode(const QString& name, double score, AbstractDialogNode* root)
	: name(name)
	, score(score)
	, root(root ? root->deepCopy() : root)
{
}

PhaseNode::PhaseNode(const PhaseNode& other)
	: name(other.name)
	, score(other.score)
	, root(other.root ? other.root->deepCopy() : other.root)
{
}

PhaseNode& PhaseNode::operator=(const PhaseNode& other)
{
	if (this != &other)
	{
		name = other.name;
		score = other.score;
		root = other.root ? other.root->deepCopy() : other.root;
	}

	return *this;
}

AbstractDialogNode* PhaseNode::shallowCopy() const
{
	return new PhaseNode(name, score, root ? root->shallowCopy() : nullptr);
}

bool PhaseNode::validate(QString& error) const
{
	if (name.trimmed().isEmpty())
	{
		error = "Имя фазы не может быть пустым";
		return false;
	}

	return true;
}

bool PhaseNode::compare(AbstractDialogNode* other) const
{
	if (other->type() != type())
	{
		return false;
	}

	return *this == *dynamic_cast<PhaseNode*>(other);
}

int PhaseNode::type() const
{
	return PhaseNode::Type;
}

bool operator==(const PhaseNode& left, const PhaseNode& right)
{
	return left.name == right.name && left.score == right.score && left.root->equalTo(right.root);
}

}
