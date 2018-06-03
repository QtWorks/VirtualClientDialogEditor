#include "clientreplicanode.h"

namespace Core
{

ClientReplicaNode::ClientReplicaNode(const QString& replica)
	: m_replica(replica)
{
}

QString ClientReplicaNode::replica() const
{
	return m_replica;
}

void ClientReplicaNode::setReplica(const QString& replica)
{
	m_replica = replica;
}

int ClientReplicaNode::type() const
{
	return ClientReplicaNode::Type;
}

bool ClientReplicaNode::validate(QString& error) const
{
	if (m_replica.trimmed().isEmpty())
	{
		error = "Реплика не может быть пустой";
		return false;
	}

	return true;
}

AbstractDialogNode* ClientReplicaNode::shallowCopy() const
{
	return new ClientReplicaNode(m_replica);
}

bool ClientReplicaNode::compareData(AbstractDialogNode* other) const
{
	Q_ASSERT(other->type() == type());
	return *this == *other->as<ClientReplicaNode>();
}

bool operator==(const ClientReplicaNode& left, const ClientReplicaNode& right)
{
	return left.replica() == right.replica();
}

}
