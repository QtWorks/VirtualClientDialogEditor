#pragma once

#include <QSet>
#include <memory>

namespace Core
{

/*class AbstractDialogNode;
typedef std::unique_ptr<AbstractDialogNode> AbstractDialogNodePtr;*/

class AbstractDialogNode
{
public:
	virtual ~AbstractDialogNode();

	enum Type
	{
		Type = 0
	};

	AbstractDialogNode& appendParent(AbstractDialogNode* parent);
	AbstractDialogNode& removeParent(AbstractDialogNode* parent);

	AbstractDialogNode& appendChild(AbstractDialogNode* child);
	AbstractDialogNode& removeChild(AbstractDialogNode* child);

	QSet<AbstractDialogNode*> parentNodes() const;
	QSet<AbstractDialogNode*> childNodes() const;

	AbstractDialogNode* deepCopy() const;
	virtual AbstractDialogNode* shallowCopy() const = 0;

	bool validate() const;
	virtual bool validate(QString& error) const = 0;
	virtual int type() const = 0;

protected:
	QSet<AbstractDialogNode*> m_parentNodes;
	QSet<AbstractDialogNode*> m_childNodes;
};

}
