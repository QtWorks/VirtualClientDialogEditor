#ifndef CLIENTREPLICAEDITOR_H
#define CLIENTREPLICAEDITOR_H

#include "core/dialog.h"
#include <QDialog>

namespace Ui {
class ClientReplicaEditor;
}

class ClientReplicaEditor
	: public QDialog
{
	Q_OBJECT

public:
	ClientReplicaEditor(const Core::ClientReplicaNode& replica, QWidget* parent = 0);
	~ClientReplicaEditor();

signals:
	void accepted(const Core::ClientReplicaNode& replica);
	void rejected();
	void changed();

private slots:
	void onReplicaChanged();
	void onSaveClicked();
	void onCancelClicked();

private:
	void updateControls();

private:
	Ui::ClientReplicaEditor* m_ui;
	Core::ClientReplicaNode m_replica;
};

#endif // CLIENTREPLICAEDITOR_H
