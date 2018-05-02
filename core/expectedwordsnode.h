#pragma once

#include "abstractdialognode.h"
#include <QString>

namespace Core
{

struct ExpectedWords
{
	ExpectedWords(const QString& words, double score)
		: words(words)
		, score(score)
	{
	}

	QString words;
	double score;
};

bool operator==(const ExpectedWords& left, const ExpectedWords& right);

class ExpectedWordsNode
	: public AbstractDialogNode
{
public:
	ExpectedWordsNode(const QList<ExpectedWords>& expectedWords);
	ExpectedWordsNode(const QList<ExpectedWords>& expectedWords, const QString& hint);

	enum Type
	{
		Type = AbstractDialogNode::Type + 3
	};

	virtual AbstractDialogNode* shallowCopy() const override;
	virtual bool validate(QString& error) const override;
	virtual bool compare(AbstractDialogNode* other) const override;
	virtual int type() const override;

	QList<ExpectedWords> expectedWords;
	bool customHint;
	QString hint;
};

bool operator==(const ExpectedWordsNode& left, const ExpectedWordsNode& right);

}
