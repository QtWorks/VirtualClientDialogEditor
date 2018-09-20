#pragma once

#include "abstractdialognode.h"
#include <QString>

namespace Core
{

class ClientReplicaNode
	: public AbstractDialogNode
{
public:
	ClientReplicaNode(const QString& replica);

	enum Type
	{
		Type = AbstractDialogNode::Type + 2
	};

	QString replica() const;
	void setReplica(const QString& replica);

	virtual int type() const override;
	virtual bool validate(QString& error) const override;

private:
	virtual AbstractDialogNode* shallowCopy() const override;
	virtual bool compareData(AbstractDialogNode* other) const override;
	virtual size_t calculateHash() const override;

private:
	QString m_replica;
};

bool operator==(const ClientReplicaNode& left, const ClientReplicaNode& right);

}
