#ifndef CLIENTREPLICANODEGRAPHICSITEM_H
#define CLIENTREPLICANODEGRAPHICSITEM_H

#include "nodegraphicsitem.h"
#include "core/dialog.h"

class ClientReplicaEditor;

class ClientReplicaNodeGraphicsItem
	: public NodeGraphicsItem
{
public:
	enum
	{
		Type = UserType + 2
	};

	ClientReplicaNodeGraphicsItem(Core::ClientReplicaNode* replica, Properties properties, QObject* parent = 0);

	virtual int type() const override;
	virtual Core::AbstractDialogNode* data() override;
	virtual const Core::AbstractDialogNode* data() const override;

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
	Core::ClientReplicaNode* m_replica;
	ClientReplicaEditor* m_editor;
};

#endif // CLIENTREPLICANODEGRAPHICSITEM_H
