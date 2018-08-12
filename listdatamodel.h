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

signals:
	void diffRecordsCountChanged(int count);
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

		notifyDiffChanged();
	}

	QMap<Data, Data> updated() const
	{
		QMap<Data, Data> result;
		for (const auto& index : m_updatedRecords.keys())
		{
			result.insert(m_updatedRecords[index].original, m_updatedRecords[index].changed);
		}
		return result;
	}

	QList<Data> deleted() const
	{
		QList<Data> result;
		for (const auto& index : m_deletedRecords)
		{
			result << get(index);
		}
		return result;
	}

	QList<Data> added() const
	{
		QList<Data> result;
		for (const auto& addedRecord : m_addedRecords)
		{
			result << addedRecord;
		}
		return result;
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

	Index append(const Data& data)
	{
		const Index id = m_freeId++;

		m_addedRecords.insert(id, data);

		notifyDiffChanged();

		return id;
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

	void update(const Index& id, const Data& data)
	{
		const auto addedIt = m_addedRecords.find(id);
		if (addedIt != m_addedRecords.end())
		{
			*addedIt = data;
			return;
		}

		const auto updatedIt = m_updatedRecords.find(id);
		if (updatedIt != m_updatedRecords.end())
		{
			(*updatedIt).changed = data;
			return;
		}

		Q_ASSERT(m_data.size() > id);
		m_updatedRecords.insert(id, { m_data[id], data });
		notifyDiffChanged();
	}

	void remove(const Index& id)
	{
		Q_ASSERT(!m_deletedRecords.contains(id));

		const auto addedIt = m_addedRecords.find(id);
		if (addedIt != m_addedRecords.end())
		{
			m_addedRecords.erase(addedIt);
			notifyDiffChanged();
			return;
		}

		const auto updatedIt = m_updatedRecords.find(id);
		if (updatedIt != m_updatedRecords.end())
		{
			m_updatedRecords.erase(updatedIt);
		}

		m_deletedRecords.append(id);
		notifyDiffChanged();
	}

	bool hasChanges(const Index& id) const
	{
		return m_addedRecords.contains(id) || m_updatedRecords.contains(id) || m_deletedRecords.contains(id);
	}

	void revert(const Index& id)
	{
		Q_ASSERT(hasChanges(id));

		const auto addedIt = m_addedRecords.find(id);
		if (addedIt != m_addedRecords.end())
		{
			m_addedRecords.erase(addedIt);
			notifyDiffChanged();
			return;
		}

		const auto updatedIt = m_updatedRecords.find(id);
		if (updatedIt != m_updatedRecords.end())
		{
			m_updatedRecords.erase(updatedIt);
			notifyDiffChanged();
			return;
		}

		Q_ASSERT(m_deletedRecords.contains(id));
		m_deletedRecords.removeAt(id);
		notifyDiffChanged();
	}

	void revertAll()
	{
		m_addedRecords.clear();
		m_updatedRecords.clear();
		m_deletedRecords.clear();

		notifyDiffChanged();
	}

private:
	void notifyDiffChanged()
	{
		emit diffRecordsCountChanged(m_addedRecords.size() + m_updatedRecords.size() + m_deletedRecords.size());
	}

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
