#include "clientreplicanode.h"

namespace Core
{

ClientReplicaNode::ClientReplicaNode(const QString& replica)
	: replica(replica)
{
}

AbstractDialogNode* ClientReplicaNode::shallowCopy() const
{
	return new ClientReplicaNode(replica);
}

bool ClientReplicaNode::validate(QString& error) const
{
	if (replica.trimmed().isEmpty())
	{
		error = "Реплика не может быть пустой";
		return false;
	}

	return true;
}

bool ClientReplicaNode::compare(AbstractDialogNode* other) const
{
	if (other->type() != type())
	{
		return false;
	}

	return *this == *dynamic_cast<ClientReplicaNode*>(other);
}

int ClientReplicaNode::type() const
{
	return ClientReplicaNode::Type;
}

bool operator==(const ClientReplicaNode& left, const ClientReplicaNode& right)
{
	return left.replica == right.replica;
}

}
