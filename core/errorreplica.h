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
		ErrorPenalty,
		FinishingExpectedWords,
		FinishingReplica
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
		for (int i = static_cast<int>(Field::ErrorReplica); i < static_cast<int>(Field::FinishingReplica); i++)
		{
			if (has(static_cast<Field>(i)))
			{
				return true;
			}
		}

		return false;
	}

	inline bool hasErrorReplica() const { return has(Field::ErrorReplica); }
	inline QString errorReplica() const { return get<QString>(Field::ErrorReplica); }
	inline void setErrorReplica(const QString& value) { set(Field::ErrorReplica, value); }

	inline bool hasErrorPenalty() const { return has(Field::ErrorPenalty); }
	inline double errorPenalty() const { return get<double>(Field::ErrorPenalty); }
	inline void setErrorPenalty(double value) { set(Field::ErrorPenalty, value); }

	inline bool hasFinishingExpectedWords() const { return has(Field::FinishingExpectedWords); }
	inline QList<QString> finishingExpectedWords() const { return get<QList<QString>>(Field::FinishingExpectedWords); }
	inline void setFinishingExpectedWords(const QList<QString>& value) { set(Field::FinishingExpectedWords, value); }

	inline bool hasFinishingReplica() const { return has(Field::FinishingReplica); }
	inline QString finishingReplica() const { return get<QString>(Field::FinishingReplica); }
	inline void setFinishingReplica(const QString& value) { set(Field::FinishingReplica, value); }
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

		if (field == Field::ErrorPenalty)
		{
			return m_errorPenalty;
		}

		if (field == Field::FinishingExpectedWords)
		{
			return m_finishingExpectedWords;
		}

		return m_finishingReplica;
	}

private:
	QVariant m_errorReplica;
	QVariant m_errorPenalty;
	QVariant m_finishingExpectedWords;
	QVariant m_finishingReplica;
};

inline bool operator==(const ErrorReplica& left, const ErrorReplica& right)
{
	const auto rank = [](const ErrorReplica& error)
	{
		return std::tie(error.m_errorReplica, error.m_errorPenalty, error.m_finishingExpectedWords, error.m_finishingReplica);
	};

	return rank(left) == rank(right);
}

inline bool operator!=(const ErrorReplica& left, const ErrorReplica& right)
{
	return !(left == right);
}

}

Q_DECLARE_METATYPE(Core::ErrorReplica)
