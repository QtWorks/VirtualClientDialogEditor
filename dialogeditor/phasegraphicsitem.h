#ifndef PHASEGRAPHICSITEM_H
#define PHASEGRAPHICSITEM_H

#include "nodegraphicsitem.h"
#include "core/phase.h"

class PhaseEditorWindow;

class PhaseGraphicsItem
	: public NodeGraphicsItem
{
	Q_OBJECT

public:
	enum
	{
		Type = UserType + 4
	};

	PhaseGraphicsItem(const Phase& phase, Properties properties, QObject* parent = 0);

	void addItem(NodeGraphicsItem* item);
	void removeItem(NodeGraphicsItem* item);

	virtual int type() const override;

	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
	virtual QString getHeaderText() const override;
	virtual QString getContentText() const override;
	virtual QBrush getHeaderBrush() const override;
	virtual void showNodeEditor() override;
	virtual NodeGraphicsItem* clone() const override;
	virtual qreal minHeight() const override;
	virtual qreal minWidth() const override;

private:
	void createEditorIfNeeded();
	void showEditor();
	void closeEditor();

private:
	Phase m_phase;
	PhaseEditorWindow* m_editor;
	QList<NodeGraphicsItem*> m_items;
};

#endif // PHASEGRAPHICSITEM_H
