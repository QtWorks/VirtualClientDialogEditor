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

int PhaseNode::type() const
{
	return PhaseNode::Type;
}

}
