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
	: expectedWords(expectedWords)
	, customHint(false)
	, hint(join(expectedWords, "; "))
{
}

ExpectedWordsNode::ExpectedWordsNode(const QList<ExpectedWords>& expectedWords, const QString& hint)
	: expectedWords(expectedWords)
	, customHint(true)
	, hint(hint)
{
}

AbstractDialogNode* ExpectedWordsNode::shallowCopy() const
{
	return customHint ? new ExpectedWordsNode(expectedWords, hint) : new ExpectedWordsNode(expectedWords);
}

bool ExpectedWordsNode::validate(QString& error) const
{
	if (expectedWords.isEmpty())
	{
		error = "Список опорных слов не может быть пустым";
		return false;
	}

	const bool hasEmptyWords = std::any_of(expectedWords.begin(), expectedWords.end(),
		[](const ExpectedWords& words)
		{
			return words.words.trimmed().isEmpty();
		});

	if (hasEmptyWords)
	{
		error = "Опорные слова не могут быть пустыми";
		return false;
	}

	if (customHint && hint.isEmpty())
	{
		error = "Подсказка не может быть пустой";
		return false;
	}

	return true;
}

bool ExpectedWordsNode::compare(AbstractDialogNode* other) const
{
	if (other->type() != type())
	{
		return false;
	}

	return *this == *dynamic_cast<ExpectedWordsNode*>(other);
}

int ExpectedWordsNode::type() const
{
	return ExpectedWordsNode::Type;
}

bool operator==(const ExpectedWordsNode& left, const ExpectedWordsNode& right)
{
	return left.expectedWords == right.expectedWords && left.customHint == right.customHint && left.hint == right.hint;
}

}
