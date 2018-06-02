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

	PhaseNode(const QString& name, double score, const QList<AbstractDialogNode*>& nodes);

	const QString& name() const;
	void setName(const QString& name);

	double score() const;
	void setScore(double score);

	const QList<AbstractDialogNode*>& nodes() const;

	virtual int type() const override;
	virtual bool validate(QString& error) const override;

private:
	virtual AbstractDialogNode* shallowCopy() const override;
	virtual bool compareData(AbstractDialogNode* other) const override;

private:
	QString m_name;
	double m_score;
	QList<AbstractDialogNode*> m_nodes;
};

bool operator==(const PhaseNode& left, const PhaseNode& right);

}
