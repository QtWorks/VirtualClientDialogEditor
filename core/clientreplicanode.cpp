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

int ClientReplicaNode::type() const
{
	return ClientReplicaNode::Type;
}

}
