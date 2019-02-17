#ifndef PHASEGRAPHICSITEM_H
#define PHASEGRAPHICSITEM_H

#include "nodegraphicsitem.h"
#include "core/dialog.h"

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

	PhaseGraphicsItem(Core::PhaseNode* phase, Core::Dialog* dialog, Properties properties, QObject* parent = 0);

	void addItem(NodeGraphicsItem* item);
	void removeItem(NodeGraphicsItem* item);
	QList<NodeGraphicsItem*> items();

	void setDialog(Core::Dialog* dialog);

	void setPrimary(bool primary);
	bool isPrimary() const;

	virtual int type() const override;
	virtual Core::AbstractDialogNode* data() override;
	virtual const Core::AbstractDialogNode* data() const override;

	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private slots:
	void onPhaseAccepted(const Core::PhaseNode& phase);
	void onPhaseAccepted(const Core::PhaseNode& phase, const Core::ErrorReplica& globalErrorReplica, const Optional<QString>& globalRepeatReplica);

private:
	virtual QString getHeaderText() const override;
	virtual QColor getHeaderTextColor() const override;
	virtual QColor getHeaderBackgroundColor() const override;

	virtual QString getContentText() const override;
	virtual QColor getContentTextColor() const override;
	virtual QColor getContentBackgroundColor() const override;

	virtual void showNodeEditor() override;
	virtual NodeGraphicsItem* clone() const override;
	virtual qreal minHeight() const override;
	virtual qreal minWidth() const override;

private:
	void createEditorIfNeeded();
	void showEditor();
	void closeEditor(bool accepted);

private:
	Core::PhaseNode* m_phase;
	Core::Dialog* m_dialog;
	PhaseEditorWindow* m_editor { nullptr };
	QList<NodeGraphicsItem*> m_items;

	bool m_primary { false };
};

#endif // PHASEGRAPHICSITEM_H
