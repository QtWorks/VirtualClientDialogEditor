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
	ExpectedWordsNode(const QList<ExpectedWords>& expectedWords, int minScore, bool forbidden);
	ExpectedWordsNode(const QList<ExpectedWords>& expectedWords, int minScore, const QString& hint, bool forbidden);

	enum Type
	{
		Type = AbstractDialogNode::Type + 3
	};

	int bestPossibleScore() const;

	const QList<ExpectedWords>& expectedWords() const;
	void setExpectedWords(const QList<ExpectedWords>& expectedWords);

	int minScore() const;
	void setMinScore(int score);

	bool customHint() const;
	void setCustomHint(bool customHint);

	const QString& hint() const;
	void setHint(const QString& hint);

	bool forbidden() const;

	virtual int type() const override;
	virtual bool validate(QString& error) const override;

private:
	virtual AbstractDialogNode* shallowCopy() const override;
	virtual bool compareData(AbstractDialogNode* other) const override;
	virtual size_t calculateHash() const override;

private:
	QList<ExpectedWords> m_expectedWords;
	int m_minScore;
	bool m_customHint;
	QString m_hint;
	bool m_forbidden;
};

bool operator==(const ExpectedWordsNode& left, const ExpectedWordsNode& right);

}
