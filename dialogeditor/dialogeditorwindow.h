#ifndef DIALOGEDITORWINDOW_H
#define DIALOGEDITORWINDOW_H

#include "core/idialogmodel.h"
#include "dialoggraphicsscene.h"
#include <QWidget>
#include <memory>

namespace Ui {
class DialogEditorWindow;
}

class DialogEditorWindow
	: public QWidget
{
	Q_OBJECT

public:
	explicit DialogEditorWindow(const Dialog& dialog, QWidget* parent = 0);
	~DialogEditorWindow();

signals:
	void dialogChanged(Dialog dialog);

public slots:
	void onNodeAdded(NodeGraphicsItem* node);
	void onNodeRemoved(NodeGraphicsItem* node);
	void onNodeSelectionChanged(NodeGraphicsItem* node, bool value);
	void onLinkAdded(ArrowLineGraphicsItem* link);
	void onLinkRemoved(ArrowLineGraphicsItem* link);

	void updateSaveControls();
	void showError(QString text);
	void hideError();

	void onConnectNodesClicked();
	void updateConnectControls();

private:
	Ui::DialogEditorWindow* m_ui;

	QGraphicsScene* m_dialogConstructorGraphicsScene;
	DialogGraphicsScene* m_dialogGraphicsScene;
	std::unique_ptr<IDialogModel> m_dialogModel;

	QVector<NodeGraphicsItem*> m_selectedNodes;
};

#endif // DIALOGEDITORWINDOW_H
