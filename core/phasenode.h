#pragma once

#include "abstractdialognode.h"
#include <QString>

namespace Core
{

class PhaseNode
	: public AbstractDialogNode
{
public:
	enum Type
	{
		Type = AbstractDialogNode::Type + 1
	};

	PhaseNode(const QString& name, double score, AbstractDialogNode* root);

	PhaseNode(const PhaseNode& other);
	PhaseNode& operator=(const PhaseNode& other);

	virtual AbstractDialogNode* shallowCopy() const override;
	virtual bool validate(QString& error) const override;
	virtual int type() const override;

	QString name;
	double score;
	AbstractDialogNode* root;
};

}
