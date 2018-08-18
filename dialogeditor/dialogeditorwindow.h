#ifndef DIALOGEDITORWINDOW_H
#define DIALOGEDITORWINDOW_H

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
	explicit DialogEditorWindow(const Core::Dialog& dialog, QWidget* parent = 0);
	~DialogEditorWindow();

signals:
	void dialogChanged(Core::Dialog dialog);

public slots:
	void onNodeAdded(NodeGraphicsItem* node);
	void onNodeRemoved(NodeGraphicsItem* node);
	void onNodeSelectionChanged(NodeGraphicsItem* node, bool value);

	void updateSaveControls();
	void showError(QString text);
	void hideError();

	void onConnectNodesClicked();
	void updateConnectControls();

	QVector<NodeGraphicsItem*> disconnectedNodes() const;
	void removeStandaloneNodes();
	void updateRemoveControls();

	void nodeAdded(NodeGraphicsItem* node);
	void nodeRemoved(NodeGraphicsItem* node);
	void nodeChanged(NodeGraphicsItem* node);

	void nodesConnected(NodeGraphicsItem* parent, NodeGraphicsItem* child);
	void nodesDisconnected(NodeGraphicsItem* parent, NodeGraphicsItem* child);

	void nodeAddedToPhase(NodeGraphicsItem* node, PhaseGraphicsItem* phase);
	void nodeRemovedFromPhase(NodeGraphicsItem* node, PhaseGraphicsItem* phase);

private:
	bool validateDialog() const;
	bool validateDialog(QString& error) const;
	QList<Core::PhaseNode> getPhases();
	QList<PhaseGraphicsItem*> getOrderedPhases();

	typedef QList<PhaseGraphicsItem*>::const_iterator PhasesListIterator;
	PhaseGraphicsItem* findFirstPhase() const;
	QList<PhaseGraphicsItem*> findNextPhase(PhaseGraphicsItem* currentPhase) const;

	Core::PhaseNode getPhaseNode(PhaseGraphicsItem* phaseItem);

	QList<Core::AbstractDialogNode*> getPhaseNodes(PhaseGraphicsItem* phaseItem);

private:
	Ui::DialogEditorWindow* m_ui;

	QGraphicsScene* m_dialogConstructorGraphicsScene;
	DialogGraphicsScene* m_dialogGraphicsScene;
	Core::Dialog m_dialog;

	QVector<NodeGraphicsItem*> m_selectedNodes;

	QVector<NodeGraphicsItem*> m_nodeItems;
	QMap<PhaseGraphicsItem*, QList<NodeGraphicsItem*>> m_nodesByPhase;
};

#endif // DIALOGEDITORWINDOW_H
