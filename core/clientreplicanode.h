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

	virtual AbstractDialogNode* shallowCopy() const override;
	virtual bool validate(QString& error) const override;
	virtual int type() const override;

	QString replica;
};

}
