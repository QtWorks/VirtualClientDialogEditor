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

	PhaseNode(const QString& name, double score, bool repeatOnInsufficientScore, const QList<AbstractDialogNode*>& nodes, const ErrorReplica& errorReplica);
	PhaseNode(const PhaseNode& other);

	const QString& name() const;
	void setName(const QString& name);

	double score() const;
	void setScore(double score);
	double bestPossibleScore() const;

	bool repeatOnInsufficientScore() const;
	void setRepeatOnInsufficientScore(bool repeatOnInsufficientScore);

	const QList<AbstractDialogNode*>& nodes() const;
	void appendNode(AbstractDialogNode* node);
	void removeNode(AbstractDialogNode* node);

	const ErrorReplica& errorReplica() const;
	ErrorReplica& errorReplica();
	void setErrorReplica(const ErrorReplica& replica);
	void resetErrorReplica();

	bool hasRepeatReplica() const;
	QString repeatReplica() const;
	void setRepeatReplica(const QString& value);
	void resetRepeatReplica();

	virtual int type() const override;
	virtual bool validate(QString& error) const override;

private:
	virtual AbstractDialogNode* shallowCopy() const override;
	virtual bool compareData(AbstractDialogNode* other) const override;
	virtual size_t calculateHash() const override;

private:
	QString m_name;
	double m_score;
	bool m_repeatOnInsufficientScore;
	QList<AbstractDialogNode*> m_nodes;

	ErrorReplica m_errorReplica;
	QVariant m_repeatReplica;
};

bool operator==(const PhaseNode& left, const PhaseNode& right);

}
