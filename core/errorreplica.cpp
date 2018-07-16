#include "errorreplica.h"

namespace Core
{

bool ErrorReplica::hasErrorReplica() const
{
	return has(Field::ErrorReplica);
}

QString ErrorReplica::errorReplica() const
{
	return get<QString>(Field::ErrorReplica);
}

void ErrorReplica::setErrorReplica(const QString& replica)
{
	set(Field::ErrorReplica, replica);
}

void ErrorReplica::resetErrorReplica()
{
	reset(Field::ErrorReplica);
}

bool ErrorReplica::hasFinishingExpectedWords() const
{
	return m_finishingExpectedWords.isValid();
}

QList<QString> ErrorReplica::finishingExpectedWords() const
{
	return m_finishingExpectedWords.value<QList<QString>>();
}

void ErrorReplica::setFinishingExpectedWords(const QList<QString>& words)
{
	m_finishingExpectedWords.setValue(words);
}

void ErrorReplica::resetFinishingExpectedWords()
{
	m_finishingExpectedWords = QVariant();
}

bool ErrorReplica::hasFinishingReplica() const
{
	return m_finishingReplica.isValid();
}

QString ErrorReplica::finishingReplica() const
{
	return m_finishingReplica.toString();
}

void ErrorReplica::setFinishingReplica(const QString& replica)
{
	m_finishingReplica = replica;
}

void ErrorReplica::resetFinishingReplica()
{
	m_finishingReplica = QVariant();
}

bool ErrorReplica::hasContinuationExpectedWords() const
{
	return m_continuationExpectedWords.isValid();
}

QList<QString> ErrorReplica::continuationExpectedWords() const
{
	return m_continuationExpectedWords.value<QList<QString>>();
}

void ErrorReplica::setContinuationExpectedWords(const QList<QString>& words)
{
	m_continuationExpectedWords.setValue(words);
}

void ErrorReplica::resetContinuationExpectedWords()
{
	m_continuationExpectedWords = QVariant();
}

}
