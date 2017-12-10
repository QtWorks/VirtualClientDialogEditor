#ifndef EXPECTEDWORDSNODEGRAPHICSITEM_H
#define EXPECTEDWORDSNODEGRAPHICSITEM_H

#include "nodegraphicsitem.h"

class ExpectedWordsEditorWindow;

class ExpectedWordsNodeGraphicsItem
	: public NodeGraphicsItem
{
public:
	enum
	{
		Type = UserType + 3
	};

	ExpectedWordsNodeGraphicsItem(const QStringList& expectedWords, Properties properties, QObject* parent = 0);

	virtual int type() const override;

private:
	virtual QString getText() const override;
	virtual QBrush getBrush() const override;
	virtual void showNodeEditor() override;
	virtual NodeGraphicsItem* clone() const override;

private:
	void createEditorIfNeeded();
	void showEditor();
	void closeEditor();

private:
	QStringList m_expectedWords;
	ExpectedWordsEditorWindow* m_editor;
};

#endif // EXPECTEDWORDSNODEGRAPHICSITEM_H
