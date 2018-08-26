#pragma once

#include <QObject>
#include <QList>
#include <QMap>

#include "logger.h"

class IListDataModel
	: public QObject
{
	Q_OBJECT

public:
	virtual ~IListDataModel() {}
};

template <typename Data>
class ListDataModel
	: public IListDataModel
{
public:
	typedef int Index;

	ListDataModel()
	{
	}

	void setData(const QList<Data>& data)
	{
		m_data = data;

		m_addedRecords.clear();
		m_updatedRecords.clear();
		m_deletedRecords.clear();

		m_freeId = m_data.size();
	}

	QList<Index> indexes() const
	{
		QList<Index> result;

		for (int i = 0; i < m_data.size(); i++)
		{
			result << i;
		}

		result << m_addedRecords.keys();

		return result;
	}

	template <typename F>
	Index findIndex(F f) const
	{
		for (const Index index : indexes())
		{
			if (f(get(index)))
			{
				return index;
			}
		}

		return -1;
	}

	Data get(const Index& id) const
	{
		const auto addedIt = m_addedRecords.find(id);
		if (addedIt != m_addedRecords.end())
		{
			return *addedIt;
		}

		const auto updatedIt = m_updatedRecords.find(id);
		if (updatedIt != m_updatedRecords.end())
		{
			return (*updatedIt).changed;
		}

		return m_data[id];
	}

private:
	QList<Data> m_data;

	struct Diff
	{
		Data original;
		Data changed;
	};

	QMap<Index, Data> m_addedRecords;
	QMap<Index, Diff> m_updatedRecords;
	QList<Index> m_deletedRecords;

	Index m_freeId;
};
