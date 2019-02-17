#pragma once

#include <QVariant>

template <typename T>
class Optional
{
public:
	Optional() = default;

	Optional<T>& operator=(const Optional<T>& other)
	{
		if (this != &other)
		{
			m_value = other.m_value;
		}
		return *this;
	}

	Optional<T>& operator=(const T& value)
	{
		m_value.setValue(value);
		return *this;
	}

	operator bool() const
	{
		return m_value.isValid();
	}

	T operator*() const
	{
		return m_value.value<T>();
	}

	void reset()
	{
		m_value = QVariant();
	}

private:
	QVariant m_value;
};

template <typename T>
inline bool operator==(const Optional<T>& left, const Optional<T>& right)
{
	return *left == *right;
}

template <typename T>
inline bool operator!=(const Optional<T>& left, const Optional<T>& right)
{
	return !(left == right);
}
