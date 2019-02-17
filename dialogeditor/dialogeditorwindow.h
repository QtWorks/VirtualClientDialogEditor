#pragma once

#include "dialoggraphicsscene.h"
#include "dialoggraphicsinfo.h"
#include "core/client.h"
#include <QWidget>
#include <QDialog>
#include <memory>

namespace Ui {
class DialogEditorWindow;
}

class DialogConstructorGraphicsScene;

class DialogEditorWindow
	: public QDialog
{
	Q_OBJECT

public:
	typedef std::function<bool(const QString&, Core::Dialog::Difficulty)> NameValidator;
	DialogEditorWindow(const Core::Client& client, const Core::Dialog& dialog,
		QList<PhaseGraphicsInfo> phasesGraphicsInfo, const NameValidator& nameValidator, QWidget* parent = 0);
	~DialogEditorWindow();

	typedef std::function<bool(const Core::Client& client, const QString&, Core::Dialog::Difficulty)> NameValidatorEx;
	void enableSaveAs(const QList<Core::Client>& clients, const Core::Client& selectedClient, NameValidatorEx nameValidator);

signals:
	void dialogModified(Core::Dialog dialog, QList<PhaseGraphicsInfo> phasesGraphicsInfo);
	void dialogCreated(Core::Client client, Core::Dialog dialog, QList<PhaseGraphicsInfo> phasesGraphicsInfo);

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

	void onPrimaryPhaseChanged(PhaseGraphicsItem* phase);

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

	DialogConstructorGraphicsScene* m_dialogConstructorGraphicsScene;
	DialogGraphicsScene* m_dialogGraphicsScene;
	Core::Dialog m_dialog;
	Core::Client m_selectedClient;
	NameValidator m_nameValidator;

	QVector<NodeGraphicsItem*> m_selectedNodes;

	QVector<NodeGraphicsItem*> m_nodeItems;
	QMap<PhaseGraphicsItem*, QList<NodeGraphicsItem*>> m_nodesByPhase;
};
