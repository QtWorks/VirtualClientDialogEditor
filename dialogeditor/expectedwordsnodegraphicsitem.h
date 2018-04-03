#ifndef EXPECTEDWORDSNODEGRAPHICSITEM_H
#define EXPECTEDWORDSNODEGRAPHICSITEM_H

#include "nodegraphicsitem.h"
#include "core/dialog.h"

class ExpectedWordsEditorWindow;

class ExpectedWordsNodeGraphicsItem
	: public NodeGraphicsItem
{
public:
	enum
	{
		Type = UserType + 3
	};

	ExpectedWordsNodeGraphicsItem(const Core::ExpectedWordsNode& expectedWords, Properties properties, QObject* parent = 0);

	virtual int type() const override;

private:
	virtual QString getHeaderText() const override;
	virtual QString getContentText() const override;
	virtual QBrush getHeaderBrush() const override;
	virtual void showNodeEditor() override;
	virtual NodeGraphicsItem* clone() const override;

private:
	void createEditorIfNeeded();
	void showEditor();
	void closeEditor();

private:
	Core::ExpectedWordsNode m_expectedWords;
	ExpectedWordsEditorWindow* m_editor;
};

#endif // EXPECTEDWORDSNODEGRAPHICSITEM_H
