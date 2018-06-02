#include "expectedwordsnode.h"

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

ExpectedWordsNode::ExpectedWordsNode(const QList<ExpectedWords>& expectedWords)
	: m_expectedWords(expectedWords)
	, m_customHint(false)
	, m_hint(join(expectedWords, "; "))
{
}

ExpectedWordsNode::ExpectedWordsNode(const QList<ExpectedWords>& expectedWords, const QString& hint)
	: m_expectedWords(expectedWords)
	, m_customHint(true)
	, m_hint(hint)
{
}

const QList<ExpectedWords>& ExpectedWordsNode::expectedWords() const
{
	return m_expectedWords;
}

void ExpectedWordsNode::setExpectedWords(const QList<ExpectedWords>& expectedWords)
{
	m_expectedWords = expectedWords;
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

	if (m_customHint && m_hint.isEmpty())
	{
		error = "Подсказка не может быть пустой";
		return false;
	}

	return true;
}

AbstractDialogNode* ExpectedWordsNode::shallowCopy() const
{
	return m_customHint ? new ExpectedWordsNode(m_expectedWords, m_hint) : new ExpectedWordsNode(m_expectedWords);
}

bool ExpectedWordsNode::compareData(AbstractDialogNode* other) const
{
	Q_ASSERT(other->type() == type());
	return *this == *other->as<ExpectedWordsNode>();
}

bool operator==(const ExpectedWordsNode& left, const ExpectedWordsNode& right)
{
	return left.expectedWords() == right.expectedWords() && left.customHint() == right.customHint() && left.hint() == right.hint();
}

}
