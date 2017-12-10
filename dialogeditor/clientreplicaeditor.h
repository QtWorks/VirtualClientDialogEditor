#ifndef CLIENTREPLICAEDITOR_H
#define CLIENTREPLICAEDITOR_H

#include <QWidget>

namespace Ui {
class ClientReplicaEditor;
}

class ClientReplicaEditor
	: public QWidget
{
	Q_OBJECT

public:
	explicit ClientReplicaEditor(const QString& replica, QWidget* parent = 0);
	~ClientReplicaEditor();

signals:
	void accepted(QString replica);
	void rejected();

private slots:
	void onSaveClicked();
	void onCancelClicked();

private:
	void updateControls();

private:
	Ui::ClientReplicaEditor* m_ui;
	QString m_replica;
};

#endif // CLIENTREPLICAEDITOR_H
