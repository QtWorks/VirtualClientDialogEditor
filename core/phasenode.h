#pragma once

#include "abstractdialognode.h"
#include "errorreplica.h"
#include <QString>
#include <QVariant>

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

	PhaseNode(const QString& name, double score, const QList<AbstractDialogNode*>& nodes, const ErrorReplica& errorReplica);
	PhaseNode(const PhaseNode& other);

	const QString& name() const;
	void setName(const QString& name);

	double score() const;
	void setScore(double score);

	const QList<AbstractDialogNode*>& nodes() const;
	void appendNode(AbstractDialogNode* node);
	void removeNode(AbstractDialogNode* node);

	const ErrorReplica& errorReplica() const;
	ErrorReplica& errorReplica();
	void setErrorReplica(const ErrorReplica& replica);
	void resetErrorReplica();

	virtual int type() const override;
	virtual bool validate(QString& error) const override;

private:
	virtual AbstractDialogNode* shallowCopy() const override;
	virtual bool compareData(AbstractDialogNode* other) const override;

private:
	QString m_name;
	double m_score;
	QList<AbstractDialogNode*> m_nodes;

	ErrorReplica m_errorReplica;
};

bool operator==(const PhaseNode& left, const PhaseNode& right);

}
