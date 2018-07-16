#pragma once

#include <QString>
#include <QList>
#include <QMetaType>
#include <QVariant>

namespace Core
{

class ErrorReplica
{
public:
	ErrorReplica() = default;
	~ErrorReplica() = default;
	ErrorReplica(const ErrorReplica& other) = default;

	enum class Field
	{
		ErrorReplica,
		FinishingExpectedWords,
		FinishingReplica,
		ContinuationExpectedWords
	};

	bool has(Field field) const
	{
		return getVariant(field).isValid();
	}

	void reset(Field field)
	{
		getVariant(field) = QVariant();
	}

	template <typename T>
	void set(Field field, const T& value)
	{
		getVariant(field).setValue(value);
	}

	template <typename T>
	T get(Field field) const
	{
		return getVariant(field).value<T>();
	}

	bool hasAnyField() const
	{
		for (int i = static_cast<int>(Field::ErrorReplica); i < static_cast<int>(Field::ContinuationExpectedWords); i++)
		{
			if (has(static_cast<Field>(i)))
			{
				return true;
			}
		}

		return false;
	}

	bool hasErrorReplica() const;
	QString errorReplica() const;
	void setErrorReplica(const QString& replica);
	void resetErrorReplica();

	bool hasFinishingExpectedWords() const;
	QList<QString> finishingExpectedWords() const;
	void setFinishingExpectedWords(const QList<QString>& words);
	void resetFinishingExpectedWords();

	bool hasFinishingReplica() const;
	QString finishingReplica() const;
	void setFinishingReplica(const QString& replica);
	void resetFinishingReplica();

	bool hasContinuationExpectedWords() const;
	QList<QString> continuationExpectedWords() const;
	void setContinuationExpectedWords(const QList<QString>& words);
	void resetContinuationExpectedWords();

	friend bool operator==(const ErrorReplica& left, const ErrorReplica& right);

private:
	const QVariant& getVariant(Field field) const
	{
		return const_cast<ErrorReplica&>(*this).getVariant(field);
	}

	QVariant& getVariant(Field field)
	{
		if (field == Field::ErrorReplica)
		{
			return m_errorReplica;
		}

		if (field == Field::FinishingExpectedWords)
		{
			return m_finishingExpectedWords;
		}

		if (field == Field::FinishingReplica)
		{
			return m_finishingReplica;
		}

		return m_continuationExpectedWords;
	}

private:
	QVariant m_errorReplica;
	QVariant m_finishingExpectedWords;
	QVariant m_finishingReplica;
	QVariant m_continuationExpectedWords;
};

inline bool operator==(const ErrorReplica& left, const ErrorReplica& right)
{
	const auto rank = [](const ErrorReplica& error)
	{
		return std::tie(error.m_errorReplica, error.m_finishingExpectedWords, error.m_finishingReplica, error.m_continuationExpectedWords);
	};

	return rank(left) == rank(right);
}

inline bool operator!=(const ErrorReplica& left, const ErrorReplica& right)
{
	return !(left == right);
}

}

Q_DECLARE_METATYPE(Core::ErrorReplica)
