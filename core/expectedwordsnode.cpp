#include "expectedwordsnode.h"
#include "hashcombine.h"

namespace Core
{

namespace
{

QString join(const QList<ExpectedWords>& expectedWordsList, const QString& delimiter)
{
	QString result;

	for (const ExpectedWords& expectedWords : expectedWordsList)
	{
		if (!result.isEmpty())
		{
			result += delimiter;
		}

		result += expectedWords.words.trimmed();
	}

	return result;
}

}

bool operator==(const ExpectedWords& left, const ExpectedWords& right)
{
	return left.words == right.words && left.score == right.score;
}

ExpectedWordsNode::ExpectedWordsNode(const QList<ExpectedWords>& expectedWords, int minScore, bool forbidden)
	: m_expectedWords(expectedWords)
	, m_minScore(minScore)
	, m_customHint(false)
	, m_hint(join(expectedWords, "; "))
	, m_forbidden(forbidden)
{
}

ExpectedWordsNode::ExpectedWordsNode(const QList<ExpectedWords>& expectedWords, int minScore, const QString& hint, bool forbidden)
	: m_expectedWords(expectedWords)
	, m_minScore(minScore)
	, m_customHint(true)
	, m_hint(hint)
	, m_forbidden(forbidden)
{
}

int ExpectedWordsNode::bestPossibleScore() const
{
	if (m_expectedWords.empty())
	{
		return 0;
	}

	int score = 0;
	for (const ExpectedWords& words : m_expectedWords)
	{
		if (words.score > 0.0)
		{
			score += words.score;
		}
	}
	return score;
}

const QList<ExpectedWords>& ExpectedWordsNode::expectedWords() const
{
	return m_expectedWords;
}

void ExpectedWordsNode::setExpectedWords(const QList<ExpectedWords>& expectedWords)
{
	m_expectedWords = expectedWords;
}

int ExpectedWordsNode::minScore() const
{
	return m_minScore;
}

void ExpectedWordsNode::setMinScore(int score)
{
	m_minScore = score;
}

bool ExpectedWordsNode::customHint() const
{
	return m_customHint;
}

void ExpectedWordsNode::setCustomHint(bool customHint)
{
	m_customHint = customHint;
}

const QString& ExpectedWordsNode::hint() const
{
	return m_hint;
}

void ExpectedWordsNode::setHint(const QString& hint)
{
	m_hint = hint;
}

bool ExpectedWordsNode::forbidden() const
{
	return m_forbidden;
}

int ExpectedWordsNode::type() const
{
	return ExpectedWordsNode::Type;
}

bool ExpectedWordsNode::validate(QString& error) const
{
	if (m_expectedWords.isEmpty())
	{
		error = "Список опорных слов не может быть пустым";
		return false;
	}

	const bool hasEmptyWords = std::any_of(m_expectedWords.begin(), m_expectedWords.end(),
		[](const ExpectedWords& words)
		{
			return words.words.trimmed().isEmpty();
		});

	if (hasEmptyWords)
	{
		error = "Опорные слова не могут быть пустыми";
		return false;
	}

	if (!m_forbidden && m_minScore > bestPossibleScore())
	{
		error = QString("Проходной балл (%1) не может быть больше суммы баллов всех опорных слов (%2)").arg(m_minScore).arg(bestPossibleScore());
		return false;
	}

	if (m_customHint && m_hint.isEmpty())
	{
		error = "Подсказка не может быть пустой";
		return false;
	}

	return true;
}

AbstractDialogNode* ExpectedWordsNode::shallowCopy() const
{
	return m_customHint ? new ExpectedWordsNode(m_expectedWords, m_minScore, m_hint, m_forbidden) : new ExpectedWordsNode(m_expectedWords, m_minScore, m_forbidden);
}

bool ExpectedWordsNode::compareData(AbstractDialogNode* other) const
{
	Q_ASSERT(other->type() == type());
	return *this == *other->as<ExpectedWordsNode>();
}

size_t ExpectedWordsNode::calculateHash() const
{
	size_t seed = 0;

	for (const ExpectedWords& words : m_expectedWords)
	{
		hashCombine(seed, words.words);
		hashCombine(seed, words.score);
	}

	hashCombine(seed, m_minScore);
	hashCombine(seed, m_customHint);
	hashCombine(seed, m_hint);
	hashCombine(seed, m_forbidden);

	return seed;
}

bool operator==(const ExpectedWordsNode& left, const ExpectedWordsNode& right)
{
	return left.expectedWords() == right.expectedWords()
		&& left.minScore() == right.minScore()
		&& left.customHint() == right.customHint()
		&& left.hint() == right.hint()
		&& left.forbidden() == right.forbidden();
}

}
