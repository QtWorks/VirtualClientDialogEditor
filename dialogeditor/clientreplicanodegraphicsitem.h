#ifndef CLIENTREPLICANODEGRAPHICSITEM_H
#define CLIENTREPLICANODEGRAPHICSITEM_H

#include "nodegraphicsitem.h"

class ClientReplicaEditor;

class ClientReplicaNodeGraphicsItem
	: public NodeGraphicsItem
{
public:
	enum
	{
		Type = UserType + 2
	};

	ClientReplicaNodeGraphicsItem(const QString& replica, Properties properties, QObject* parent = 0);

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
	QString m_replica;
	ClientReplicaEditor* m_editor;
};

#endif // CLIENTREPLICANODEGRAPHICSITEM_H
