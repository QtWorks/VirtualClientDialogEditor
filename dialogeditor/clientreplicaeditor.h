#ifndef CLIENTREPLICAEDITOR_H
#define CLIENTREPLICAEDITOR_H

#include "core/dialog.h"
#include <QWidget>

namespace Ui {
class ClientReplicaEditor;
}

class ClientReplicaEditor
	: public QWidget
{
	Q_OBJECT

public:
	explicit ClientReplicaEditor(const Core::ClientReplicaNode& replica, QWidget* parent = 0);
	~ClientReplicaEditor();

signals:
	void accepted(Core::ClientReplicaNode replica);
	void rejected();

private slots:
	void onSaveClicked();
	void onCancelClicked();

private:
	void updateControls();

private:
	Ui::ClientReplicaEditor* m_ui;
	Core::ClientReplicaNode m_replica;
};

#endif // CLIENTREPLICAEDITOR_H
