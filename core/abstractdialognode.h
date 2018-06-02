#pragma once

#include <QSet>
#include <memory>

namespace Core
{

class AbstractDialogNode
{
public:
	typedef QString Id;

	AbstractDialogNode();
	virtual ~AbstractDialogNode();

	enum Type
	{
		Type = 0
	};

	Id id() const;
	void setId(Id id);

	QSet<Id> parentNodes() const;
	void appendParent(const Id& id);
	void removeParent(const Id& id);

	QSet<Id> childNodes() const;
	void appendChild(const Id& id);
	void removeChild(const Id& id);

	AbstractDialogNode* clone() const;

	bool validate() const;
	virtual bool validate(QString& error) const = 0;
	virtual int type() const = 0;

	bool compare(AbstractDialogNode* other) const;

	template <typename T>
	T* as()
	{
		return type() == T::Type ? static_cast<T*>(this) : nullptr;
	}

	template <typename T>
	const T* as(const AbstractDialogNode* node)
	{
		return type() == T::Type ? static_cast<const T*>(this) : nullptr;
	}

protected:
	QSet<Id> m_parentNodes;
	QSet<Id> m_childNodes;

private:
	virtual AbstractDialogNode* shallowCopy() const = 0;
	virtual bool compareData(AbstractDialogNode* other) const = 0;

private:
	Id m_id;
};

bool operator==(const AbstractDialogNode& left, const AbstractDialogNode& right);

template <typename Iterator>
Iterator findNodeById(Iterator begin, Iterator end, const AbstractDialogNode::Id& id)
{
	return std::find_if(begin, end, [&id](const AbstractDialogNode* node) { return node->id() == id; });
}

template <typename Container>
typename Container::const_iterator findNodeById(const Container& container, const AbstractDialogNode::Id& id)
{
	return findNodeById(container.begin(), container.end(), id);
}

template <typename Container>
typename Container::iterator findNodeById(Container& container, const AbstractDialogNode::Id& id)
{
	return findNodeById(container.begin(), container.end(), id);
}

}
