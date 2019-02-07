#include "dialogeditorwindow.h"
#include "ui_dialogeditorwindow.h"
#include "zoomablegraphicsview.h"
#include "dialogconstructorgraphicsscene.h"
#include "dialoggraphicsscene.h"

#include "phasegraphicsitem.h"
#include "clientreplicanodegraphicsitem.h"
#include "expectedwordsnodegraphicsitem.h"
#include "arrowlinegraphicsitem.h"
#include "saveasdialog.h"
#include "groupsdialog.h"

#include "logger.h"
#include <QPushButton>
#include <QMessageBox>

#include <set>

namespace
{

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

QString nodeType(Core::AbstractDialogNode* node)
{
	if (node->type() == Core::ClientReplicaNode::Type)
	{
		return "ClientReplica";
	}

	if (node->type() == Core::ExpectedWordsNode::Type)
	{
		return "ExpectedWords";
	}

	return "Phase";
}

QString nodeType(NodeGraphicsItem* node)
{
	return nodeType(node->data());
}

bool nodesAlreadyConnected(NodeGraphicsItem* parent, NodeGraphicsItem* child)
{
	const auto outcomingLinks = parent->outcomingLinks();
	const auto existingLinkIt = std::find_if(outcomingLinks.begin(), outcomingLinks.end(),
		[parent, child](ArrowLineGraphicsItem* link)
		{
			return link->isConnectingNodes() && link->parentNode() == parent && link->childNode() == child;
		});

	return existingLinkIt != outcomingLinks.end();
}

QList<Core::AbstractDialogNode*> gatherNodes(const QMap<PhaseGraphicsItem*, QList<NodeGraphicsItem*>>& dialog)
{
	QList<Core::AbstractDialogNode*> result;

	for (PhaseGraphicsItem* phase : dialog.keys())
	{
		for (NodeGraphicsItem* node : dialog[phase])
		{
			result << node->data()->clone(false);
		}
	}

	return result;
}

typedef QList<Core::AbstractDialogNode*> NodesPath;

QList<NodesPath> findPathsBetweenNodes(Core::AbstractDialogNode* childNode, Core::AbstractDialogNode* parentNode, const QList<Core::AbstractDialogNode*>& nodes, bool compareWithSelf = true)
{
	LOG << "searching path to root from " << childNode->id();

	if (compareWithSelf && childNode == parentNode)
	{
		LOG << childNode->id() << " is target node (" << parentNode->id() << ")";
		return { { childNode } };
	}

	QList<NodesPath> path;

	for (const Core::AbstractDialogNode::Id& parentId : childNode->parentNodes())
	{
		const auto parentNodeIt = Core::findNodeById(nodes.begin(), nodes.end(), parentId);
		if (parentNodeIt == nodes.end())
		{
			LOG << childNode->id() << " is target node (" << parentNode->id() << ")";
			return { { childNode } };
		}

		QList<NodesPath> pathToParent = findPathsBetweenNodes(*parentNodeIt, parentNode, nodes);
		for (NodesPath& path : pathToParent)
		{
			path.prepend(childNode);
		}

		LOG << "found " << pathToParent.length() << " paths to " << childNode->id();

		path.append(pathToParent);
	}

	return path;
}

bool hasCycles(Core::AbstractDialogNode* childNode, const QList<Core::AbstractDialogNode*>& nodes)
{
	QList<NodesPath> paths = findPathsBetweenNodes(childNode, childNode, nodes, false);

	LOG << "Found " << paths.size() << " potential paths to " << childNode->id() << " from itself";

	for (const NodesPath& path : paths)
	{
		QStringList nodes;
		std::transform(path.begin(), path.end(), std::back_inserter(nodes), [](Core::AbstractDialogNode* node){ return node->id(); });

		LOG << "Path: " << nodes.join("->");
	}

	paths.erase(
		std::remove_if(paths.begin(), paths.end(), [childNode](const NodesPath& path) { return path.length() == 1 && path[0] == childNode; }),
		paths.end());

	return !paths.isEmpty();
}

QList<PhaseGraphicsInfo> getPhasesGraphicsInfo(QList<PhaseGraphicsItem*> phases)
{
	QList<PhaseGraphicsInfo> result;

	for (PhaseGraphicsItem* phaseGraphicsItem : phases)
	{
		PhaseGraphicsInfo phaseGraphicsInfo;

		auto* phaseNode = phaseGraphicsItem->data()->as<Core::PhaseNode>();

		phaseGraphicsInfo.name = phaseNode->name();
		phaseGraphicsInfo.position = phaseGraphicsItem->pos();
		phaseGraphicsInfo.size = phaseGraphicsItem->boundingRect().size();
		phaseGraphicsInfo.nodes = ([&]() -> QList<NodeGraphicsInfo>
		{
			QList<NodeGraphicsInfo> nodes;

			for (NodeGraphicsItem* nodeGraphicsItem : phaseGraphicsItem->items())
			{
				NodeGraphicsInfo nodeGraphicsInfo;
				nodeGraphicsInfo.id = nodeGraphicsItem->data()->id();
				nodeGraphicsInfo.position = nodeGraphicsItem->pos();
				nodeGraphicsInfo.size = nodeGraphicsItem->boundingRect().size();
				nodes.append(nodeGraphicsInfo);
			}

			return nodes;
		})();

		result.append(phaseGraphicsInfo);
	}

	return result;
}

}

DialogEditorWindow::DialogEditorWindow(const Core::Client& client, const Core::Dialog& dialog, QList<PhaseGraphicsInfo> phasesGraphicsInfo,
	const NameValidator& nameValidator, QWidget* parent)
	: QDialog(parent)
	, m_ui(new Ui::DialogEditorWindow)
	, m_dialogConstructorGraphicsScene(new DialogConstructorGraphicsScene(this))
	, m_dialogGraphicsScene(new DialogGraphicsScene(this))
	, m_dialog(dialog)
	, m_nameValidator(nameValidator)
{
	m_ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose, true);
	setModal(true);

	m_ui->nameEdit->setText(dialog.name);
	connect(m_ui->nameEdit, &QLineEdit::textEdited, [this](const QString& name) { m_dialog.name = name; });
	connect(m_ui->nameEdit, &QLineEdit::textEdited, this, &DialogEditorWindow::updateSaveControls);

	m_ui->difficultyComboBox->addItems(Core::Dialog::availableDifficulties());
	const int index = m_ui->difficultyComboBox->findText(Core::Dialog::difficultyToString(dialog.difficulty), Qt::MatchCaseSensitive);
	m_ui->difficultyComboBox->setCurrentIndex(index);
	connect(m_ui->difficultyComboBox, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
		[this](const QString& difficulty)
		{
			m_dialog.difficulty = Core::Dialog::difficultyFromString(difficulty);
			updateSaveControls();
		});

	QList<Core::Group> clientGroups = client.groups;
	connect(m_ui->selectGroupsButton, &QPushButton::clicked, [this, clientGroups]()
	{
		GroupsDialog* groupsDialog = new GroupsDialog(clientGroups, m_dialog.groups, this);

		connect(groupsDialog, &GroupsDialog::accepted, [this](const QList<QString>& checkedGroups)
		{
			m_dialog.groups = checkedGroups;
		});

		groupsDialog->show();
	});

	m_ui->noteTextEdit->setText(dialog.note);
	connect(m_ui->noteTextEdit, &QTextEdit::textChanged, [this]() { m_dialog.note = m_ui->noteTextEdit->toPlainText().trimmed(); });
	connect(m_ui->noteTextEdit, &QTextEdit::textChanged, this, &DialogEditorWindow::updateSaveControls);

	m_ui->successRatioLineEdit->setText(QString::number(dialog.successRatio));
	m_ui->successRatioLineEdit->setValidator(new QIntValidator(0, 100));
	connect(m_ui->successRatioLineEdit, &QLineEdit::textEdited, [this](const QString& successRatio) { m_dialog.successRatio = successRatio.toDouble(); });
	connect(m_ui->successRatioLineEdit, &QLineEdit::textEdited, this, &DialogEditorWindow::updateSaveControls);

	QIcon warningIcon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
	QPixmap warningPixmap = warningIcon.pixmap(QSize(16, 16));
	m_ui->errorIconLabel->setPixmap(warningPixmap);

	connect(m_ui->saveButton, &QPushButton::clicked, [this]()
	{
		Q_ASSERT(validateDialog());

		QList<PhaseGraphicsInfo> graphicsInfo = getPhasesGraphicsInfo(getOrderedPhases());

		m_dialog.phases = getPhases();

		emit dialogModified(m_dialog, graphicsInfo);

		close();
	});

	m_ui->saveAsButton->hide();

	connect(m_ui->closeButton, &QPushButton::clicked, [this]()
	{
		close();
	});

	m_ui->splitter->setStretchFactor(0, 0);
	m_ui->splitter->setStretchFactor(1, 1);

	m_ui->constructorGraphicsView->setScene(m_dialogConstructorGraphicsScene);

	const int padding = 10;
	QRectF constructorSceneRect = m_ui->constructorGraphicsView->sceneRect();
	constructorSceneRect.setWidth(constructorSceneRect.width() + padding);
	constructorSceneRect.setX(constructorSceneRect.x() - padding);
	m_ui->constructorGraphicsView->setSceneRect(constructorSceneRect);

	connect(m_ui->connectNodesButton, &QPushButton::clicked, this, &DialogEditorWindow::onConnectNodesClicked);
	connect(m_ui->removeStandaloneNodesButton, &QPushButton::clicked, this, &DialogEditorWindow::removeStandaloneNodes);

	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeSelectionChanged, this, &DialogEditorWindow::onNodeSelectionChanged);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeSelectionChanged, this, &DialogEditorWindow::updateConnectControls);

	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeAdded, this, &DialogEditorWindow::nodeAdded);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeAdded, this, &DialogEditorWindow::updateSaveControls);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeAdded, this, &DialogEditorWindow::updateRemoveControls);

	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeRemoved, this, &DialogEditorWindow::nodeRemoved);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeRemoved, this, &DialogEditorWindow::updateSaveControls);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeRemoved, this, &DialogEditorWindow::updateRemoveControls);

	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeChanged, this, &DialogEditorWindow::nodeChanged);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeChanged, this, &DialogEditorWindow::updateSaveControls);

	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeAddedToPhase, this, &DialogEditorWindow::nodeAddedToPhase);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeAddedToPhase, this, &DialogEditorWindow::updateSaveControls);

	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeRemovedFromPhase, this, &DialogEditorWindow::nodeRemovedFromPhase);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeRemovedFromPhase, this, &DialogEditorWindow::updateSaveControls);

	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodesConnected, this, &DialogEditorWindow::nodesConnected);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodesConnected, this, &DialogEditorWindow::updateSaveControls);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodesConnected, this, &DialogEditorWindow::updateRemoveControls);

	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodesDisconnected, this, &DialogEditorWindow::nodesDisconnected);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodesDisconnected, this, &DialogEditorWindow::updateSaveControls);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodesDisconnected, this, &DialogEditorWindow::updateRemoveControls);

	m_dialogGraphicsScene->setDialog(&m_dialog, phasesGraphicsInfo);

	m_ui->dialogGraphicsView->setScene(m_dialogGraphicsScene);
	m_ui->dialogGraphicsView->setMinRatio(50.0);
	m_ui->dialogGraphicsView->setMaxRatio(150.0);

	updateSaveControls();
	updateConnectControls();
	updateRemoveControls();
}

DialogEditorWindow::~DialogEditorWindow()
{
	delete m_ui;
}

void DialogEditorWindow::enableSaveAs(const QList<Core::Client>& clients, const Core::Client& selectedClient, NameValidatorEx nameValidator)
{
	m_ui->saveAsButton->show();

	connect(m_ui->saveAsButton, QPushButton::clicked, [=]()
	{
		SaveAsDialog::NameValidator validator = [&](const Core::Client& client, const QString& name)
		{
			return nameValidator(client, name, m_dialog.difficulty);
		};

		SaveAsDialog* dialog = new SaveAsDialog(clients, selectedClient, validator, this);
		dialog->show();

		connect(dialog, &SaveAsDialog::accepted, [this](Core::Client client, QString newDialogName)
		{
			QList<PhaseGraphicsInfo> graphicsInfo = getPhasesGraphicsInfo(getOrderedPhases());
			m_dialog.name = newDialogName;
			m_dialog.phases = getPhases();

			emit dialogCreated(client, m_dialog, graphicsInfo);
			close();
		});
	});
}

void DialogEditorWindow::onNodeAdded(NodeGraphicsItem* /*node*/)
{
}

void DialogEditorWindow::onNodeRemoved(NodeGraphicsItem* node)
{
	m_selectedNodes.removeOne(node);
}

void DialogEditorWindow::onNodeSelectionChanged(NodeGraphicsItem* node, bool value)
{
	const auto it = std::find(m_selectedNodes.begin(), m_selectedNodes.end(), node);
	if (it == m_selectedNodes.end())
	{
		Q_ASSERT(value);
		m_selectedNodes.append(node);
	}
	else
	{
		Q_ASSERT(!value);
		m_selectedNodes.erase(it);
	}
}

void DialogEditorWindow::updateSaveControls()
{
	QString error;
	if (!validateDialog(error))
	{
		showError(error);
	}
	else
	{
		hideError();
	}
}

void DialogEditorWindow::showError(QString text)
{
	m_ui->errorIconLabel->show();

	m_ui->errorTextLabel->setText(text);
	m_ui->errorTextLabel->show();

	m_ui->saveButton->setEnabled(false);
	m_ui->saveAsButton->setEnabled(false);
}

void DialogEditorWindow::hideError()
{
	m_ui->errorIconLabel->hide();

	m_ui->errorTextLabel->setText("");
	m_ui->errorTextLabel->hide();

	m_ui->saveButton->setEnabled(true);
	m_ui->saveAsButton->setEnabled(true);
}

void DialogEditorWindow::onConnectNodesClicked()
{
	Q_ASSERT(m_selectedNodes.size() == 2);

	NodeGraphicsItem* parentNode = m_selectedNodes[0];
	NodeGraphicsItem* childNode = m_selectedNodes[1];

	m_dialogGraphicsScene->connectNodes(parentNode, childNode);

	parentNode->setSelected(false);
	childNode->setSelected(false);
}

QVector<NodeGraphicsItem*> DialogEditorWindow::disconnectedNodes() const
{
	QVector<NodeGraphicsItem*> nodes;

	std::copy_if(m_nodeItems.begin(), m_nodeItems.end(), std::back_inserter(nodes),
		[](NodeGraphicsItem* node)
		{
			if (node->type() == ClientReplicaNodeGraphicsItem::Type ||
				node->type() == ExpectedWordsNodeGraphicsItem::Type)
			{
				return node->data()->parentNodes().isEmpty() && node->data()->childNodes().isEmpty();
			}

			return false;
		});

	return nodes;
}

void DialogEditorWindow::removeStandaloneNodes()
{
	QVector<NodeGraphicsItem*> nodes = disconnectedNodes();
	if (nodes.isEmpty())
	{
		return;
	}

	QMessageBox messageBox(QMessageBox::Question,
		"Удаление блоков",
		"Вы действительно хотите удалить " + QString::number(nodes.size()) + " блоков без стрелок?",
		QMessageBox::Yes | QMessageBox::No,
		this);
	messageBox.setButtonText(QMessageBox::Yes, tr("Да"));
	messageBox.setButtonText(QMessageBox::No, tr("Нет"));

	const int answer = messageBox.exec();
	if (answer != QMessageBox::Yes)
	{
		return;
	}

	for (NodeGraphicsItem* node : nodes)
	{
		PhaseGraphicsItem* phaseItem = node->getPhase();
		if (phaseItem)
		{
			phaseItem->removeItem(node);
		}

		m_dialogGraphicsScene->removeNodeFromScene(node);
	}
}

void DialogEditorWindow::updateRemoveControls()
{
	QVector<NodeGraphicsItem*> nodes = disconnectedNodes();
	m_ui->removeStandaloneNodesButton->setEnabled(nodes.size() > 0);
}

void DialogEditorWindow::updateConnectControls()
{
	if (m_selectedNodes.size() != 2)
	{
		m_ui->connectNodesButton->setEnabled(false);
		return;
	}

	NodeGraphicsItem* parentNode = m_selectedNodes[0];
	NodeGraphicsItem* childNode = m_selectedNodes[1];

	if (m_dialog.difficulty == Core::Dialog::Difficulty::Easy &&
		(!parentNode->outcomingLinks().isEmpty() || !childNode->incomingLinks().isEmpty()))
	{
		m_ui->connectNodesButton->setEnabled(false);
		return;
	}

	if (nodesAlreadyConnected(parentNode, childNode))
	{
		m_ui->connectNodesButton->setEnabled(false);
		return;
	}

	if (parentNode->type() == PhaseGraphicsItem::Type || childNode->type() == PhaseGraphicsItem::Type)
	{
		m_ui->connectNodesButton->setEnabled(false);
		return;
	}

	// TODO: optimize
	/*const Core::AbstractDialogNode::Id childNodeId = childNode->data()->id();
	const Core::AbstractDialogNode::Id parentNodeId = parentNode->data()->id();

	QList<Core::AbstractDialogNode*> nodes = gatherNodes(m_nodesByPhase);
	auto childNodeIt = Core::findNodeById(nodes.begin(), nodes.end(), childNodeId);
	auto parentNodeIt = Core::findNodeById(nodes.begin(), nodes.end(), parentNodeId);
	(*childNodeIt)->appendParent(parentNodeId);
	(*parentNodeIt)->appendChild(childNodeId);
	if (hasCycles(*childNodeIt, nodes))
	{
		m_ui->connectNodesButton->setEnabled(false);
		return;
	}*/

	m_ui->connectNodesButton->setEnabled(
		(parentNode->type() == ClientReplicaNodeGraphicsItem::Type && childNode->type() == ExpectedWordsNodeGraphicsItem::Type) ||
		(parentNode->type() == ExpectedWordsNodeGraphicsItem::Type && childNode->type() == ClientReplicaNodeGraphicsItem::Type) ||
		(parentNode->type() == ClientReplicaNodeGraphicsItem::Type && childNode->type() == ClientReplicaNodeGraphicsItem::Type)
	);
}

void DialogEditorWindow::nodeAdded(NodeGraphicsItem* node)
{
	LOG << ARG2(nodeType(node), "node");

	Q_ASSERT(!m_nodeItems.contains(node));
	m_nodeItems.push_back(node);

	if (node->type() == PhaseGraphicsItem::Type)
	{
		PhaseGraphicsItem* phaseItem = qgraphicsitem_cast<PhaseGraphicsItem*>(node);
		m_nodesByPhase.insert(phaseItem, {});

		const Core::PhaseNode* phaseNode = phaseItem->data()->as<const Core::PhaseNode>();
		const auto phaseIt = std::find_if(m_dialog.phases.begin(), m_dialog.phases.end(),
			[&phaseNode](const Core::PhaseNode& phase){ return phase.id() == phaseNode->id(); });
		if (phaseIt == m_dialog.phases.end())
		{
			m_dialog.phases.append(*phaseNode);
		}
	}
}

void DialogEditorWindow::nodeRemoved(NodeGraphicsItem* node)
{
	LOG << ARG2(nodeType(node), "node");

	Q_ASSERT(m_nodeItems.contains(node));
	m_nodeItems.removeOne(node);

	if (node->type() == PhaseGraphicsItem::Type)
	{
		PhaseGraphicsItem* phaseItem = qgraphicsitem_cast<PhaseGraphicsItem*>(node);
		m_nodesByPhase.remove(phaseItem);

		const Core::PhaseNode* phaseNode = phaseItem->data()->as<const Core::PhaseNode>();

		const auto it = std::find_if(m_dialog.phases.begin(), m_dialog.phases.end(),
			[phaseNode](const Core::PhaseNode& phase) { return phase.id() == phaseNode->id(); });
		Q_ASSERT(it != m_dialog.phases.end());
		const int phaseIndex = std::distance(it, m_dialog.phases.begin());
		m_dialog.phases.removeAt(phaseIndex);
	}

	if (m_selectedNodes.contains(node))
	{
		node->setSelected(false);
	}
}

void DialogEditorWindow::nodeChanged(NodeGraphicsItem* node)
{
	if (node->type() != PhaseGraphicsItem::Type)
	{
		return;
	}

	PhaseGraphicsItem* phaseItem = qgraphicsitem_cast<PhaseGraphicsItem*>(node);
	const Core::PhaseNode* phaseNode = phaseItem->data()->as<const Core::PhaseNode>();

	auto it = std::find_if(m_dialog.phases.begin(), m_dialog.phases.end(),
		[phaseNode](const Core::PhaseNode& phase) { return phase.id() == phaseNode->id(); });
	Q_ASSERT(it != m_dialog.phases.end());
	*it = *phaseNode;
}

void DialogEditorWindow::nodesConnected(NodeGraphicsItem* parent, NodeGraphicsItem* child)
{
	LOG << ARG2(nodeType(parent), "parent") << ARG2(nodeType(child), "child");

	Q_ASSERT(m_nodeItems.contains(parent));
	Q_ASSERT(m_nodeItems.contains(child));

	Core::AbstractDialogNode* parentNode = parent->data();
	Core::AbstractDialogNode* childNode = child->data();

	parentNode->appendChild(childNode->id());
	childNode->appendParent(parentNode->id());
}

void DialogEditorWindow::nodesDisconnected(NodeGraphicsItem* parent, NodeGraphicsItem* child)
{
	LOG << ARG2(nodeType(parent), "parent") << ARG2(nodeType(child), "child");

	Q_ASSERT(m_nodeItems.contains(parent));
	Q_ASSERT(m_nodeItems.contains(child));

	Core::AbstractDialogNode* parentNode = parent->data();
	Core::AbstractDialogNode* childNode = child->data();

	Q_ASSERT(parentNode->childNodes().contains(childNode->id()));
	parentNode->removeChild(childNode->id());
	childNode->removeParent(parentNode->id());
}

void DialogEditorWindow::nodeAddedToPhase(NodeGraphicsItem* node, PhaseGraphicsItem* phase)
{
	LOG << ARG2(nodeType(node), "node") << ARG2(phase->sceneBoundingRect(), "PhaseSBR") << ARG2(node->sceneBoundingRect(), "NodeSBR");
	Q_ASSERT(m_nodesByPhase.contains(phase));

	m_nodesByPhase[phase].append(node);
	phase->data()->as<Core::PhaseNode>()->appendNode(node->data());
}

void DialogEditorWindow::nodeRemovedFromPhase(NodeGraphicsItem* node, PhaseGraphicsItem* phase)
{
	LOG << ARG2(nodeType(node), "node");
	Q_ASSERT(m_nodesByPhase.contains(phase));

	m_nodesByPhase[phase].removeOne(node);
	phase->data()->as<Core::PhaseNode>()->removeNode(node->data());
}

bool DialogEditorWindow::validateDialog() const
{
	QString error;
	return validateDialog(error);
}

bool DialogEditorWindow::validateDialog(QString& error) const
{
	if (m_dialog.name.trimmed().isEmpty())
	{
		error = "Имя диалога не может быть пустым";
		return false;
	}

	if (!m_nameValidator(m_dialog.name, m_dialog.difficulty))
	{
		error = "Имя диалога должно быть уникальным";
		return false;
	}

	const Core::ErrorReplica& errorReplica = m_dialog.errorReplica;	
	if (errorReplica.hasErrorReplica() && errorReplica.errorReplica().trimmed().isEmpty())
	{
		error = "Ошибочная реплика не может быть пустой";
		return false;
	}

	if (errorReplica.hasErrorPenalty() && errorReplica.errorPenalty() <= 0.0)
	{
		error = "Количество штрафных баллов должно быть больше 0";
		return false;
	}

	if (errorReplica.hasFinishingReplica() && errorReplica.finishingReplica().trimmed().isEmpty())
	{
		error = "Завершающая реплика не может быть пустой";
		return false;
	}

	if (errorReplica.hasFinishingExpectedWords() && errorReplica.finishingExpectedWords().isEmpty())
	{
		error = "Завершающие опорные слова не могут быть пустыми";
		return false;
	}

	QVector<Core::AbstractDialogNode*> nodes;
	std::transform(m_nodeItems.begin(), m_nodeItems.end(), std::back_inserter(nodes), [](NodeGraphicsItem* item) { return item->data(); });

	if (nodes.isEmpty())
	{
		error = "Диалог не может быть пустым";
		return false;
	}

	const int nodesWithoutParents = std::count_if(nodes.begin(), nodes.end(),
		[](Core::AbstractDialogNode* node)
		{
			if (node->type() == Core::ClientReplicaNode::Type ||
				node->type() == Core::ExpectedWordsNode::Type)
			{
				return node->parentNodes().isEmpty();
			}

			return false;
		});

	if (nodesWithoutParents > 1)
	{
		error = "Должен быть только 1 узел без входящих стрелок (" + QString::number(nodesWithoutParents) + ")";
		return false;
	}

	if (m_dialog.difficulty == Core::Dialog::Difficulty::Easy)
	{
		const int nodesWithoutChilds = std::count_if(nodes.begin(), nodes.end(),
			[](Core::AbstractDialogNode* node)
			{
				if (node->type() == Core::ClientReplicaNode::Type ||
					node->type() == Core::ExpectedWordsNode::Type)
				{
					return node->childNodes().isEmpty();
				}

				return false;
			});

		if (nodesWithoutChilds > 1)
		{
			error = "Должен быть только 1 узел без выходящих стрелок (" + QString::number(nodesWithoutChilds) + ")";
			return false;
		}
	}

	if (m_dialog.difficulty == Core::Dialog::Difficulty::Easy)
	{
		const auto incorrectLinksNodeIt = std::find_if(nodes.begin(), nodes.end(),
			[](Core::AbstractDialogNode* node)
			{
				if (node->type() != Core::ClientReplicaNode::Type &&
					node->type() != Core::ExpectedWordsNode::Type)
				{
					return false;
				}

				return node->parentNodes().size() > 1 || node->childNodes().size() > 1;
			});
		if (incorrectLinksNodeIt != nodes.end())
		{
			error = "Каждый узел должен иметь не больше 1 входящей и 1 выходящей стрелки";
			return false;
		}
	}

	const auto invalidNodeIt = std::find_if(nodes.begin(), nodes.end(),
		[](Core::AbstractDialogNode* node)
		{
			return !node->validate();
		});
	if (invalidNodeIt != nodes.end())
	{
		error = "Каждый узел должен быть заполнен корректно";
		return false;
	}

	const auto emptyPhaseIt = std::find_if(m_nodesByPhase.begin(), m_nodesByPhase.end(),
		[](const QList<NodeGraphicsItem*>& nodesByPhase)
		{
			return nodesByPhase.isEmpty();
		});
	if (emptyPhaseIt != m_nodesByPhase.end())
	{
		error = "Фазы не могут быть пустыми";
		return false;
	}

	const auto nodeOutsidePhaseIt = std::find_if(nodes.begin(), nodes.end(),
		[this](Core::AbstractDialogNode* node)
		{
			if (node->type() == Core::PhaseNode::Type)
			{
				return false;
			}

			for (const auto& phase : m_nodesByPhase.keys())
			{
				const auto& nodes = m_nodesByPhase.value(phase);
				const auto existingNodeIt = std::find_if(nodes.begin(), nodes.end(), [node](NodeGraphicsItem* item) { return item->data() == node; });
				if (existingNodeIt != nodes.end())
				{
					return false;
				}
			}

			return true;
		});
	if (nodeOutsidePhaseIt != nodes.end())
	{
		error = "Каждый узел должен находиться в какой-либо фазе";
		return false;
	}

	// TODO: use phases from dialog, just sort them
	PhaseGraphicsItem* phase = findFirstPhase();
	if (phase)
	{
		while (phase)
		{
			const QList<PhaseGraphicsItem*> nextPhases = findNextPhase(phase);
			if (nextPhases.empty())
			{
				phase = nullptr;
				continue;
			}

			const bool allPhasesAreSame = std::set<PhaseGraphicsItem*>(nextPhases.begin(), nextPhases.end()).size() <= 1;
			if (!allPhasesAreSame)
			{
				error = "Ветвление по фазам недопустимо (" + phase->data()->as<Core::PhaseNode>()->name() + ")";
				return false;
			}
			phase = *nextPhases.begin();
		}
	}
	else
	{
		error = "wut";
		return false;
	}

	return true;
}

QList<Core::PhaseNode> DialogEditorWindow::getPhases()
{
	QList<Core::PhaseNode> result;
	QList<PhaseGraphicsItem*> orderedPhases = getOrderedPhases();

	for (PhaseGraphicsItem* phaseItem : orderedPhases)
	{
		result.append(getPhaseNode(phaseItem));
	}

	return result;
}

QList<Core::AbstractDialogNode*> getPhaseNodes(const QList<NodeGraphicsItem*>& items)
{
	QList<Core::AbstractDialogNode*> nodes;
	std::transform(items.begin(), items.end(), std::back_inserter(nodes),
		[](NodeGraphicsItem* item) { return item->data(); });
	return nodes;
}

QList<PhaseGraphicsItem*> DialogEditorWindow::getOrderedPhases()
{
	QList<PhaseGraphicsItem*> result;

	PhaseGraphicsItem* phase = findFirstPhase();
	Q_ASSERT(phase != nullptr);

	while (phase != nullptr)
	{
		result.append(phase);

		Core::PhaseNode* phaseNode = phase->data()->as<Core::PhaseNode>();
		LOG << "Appending phase " << phaseNode->name();

		const QList<PhaseGraphicsItem*> nextPhases = findNextPhase(phase);
		if (nextPhases.empty())
		{
			phase = nullptr;
			continue;
		}

		const bool allPhasesAreSame = std::set<PhaseGraphicsItem*>(nextPhases.begin(), nextPhases.end()).size() <= 1;
		Q_ASSERT(allPhasesAreSame);
		phase = *nextPhases.begin();
	}

	return result;
}

PhaseGraphicsItem* DialogEditorWindow::findFirstPhase() const
{
	const QList<PhaseGraphicsItem*> phasesList = m_nodesByPhase.keys();

	auto it = std::find_if(phasesList.cbegin(), phasesList.cend(),
		[this](PhaseGraphicsItem* phaseItem)
		{
			const QList<NodeGraphicsItem*>& phaseItems = m_nodesByPhase[phaseItem];
			return std::any_of(phaseItems.begin(), phaseItems.end(),
				[](NodeGraphicsItem* item) { return item->data()->parentNodes().empty(); });
		});
	return *it;
}

QList<PhaseGraphicsItem*> DialogEditorWindow::findNextPhase(PhaseGraphicsItem* currentPhase) const
{
	const QList<Core::AbstractDialogNode*> nodes = ::getPhaseNodes(m_nodesByPhase[currentPhase]);

	QList<Core::AbstractDialogNode::Id> nextPhaseItems;
	for (Core::AbstractDialogNode* node : nodes)
	{
		const QSet<Core::AbstractDialogNode::Id> childNodes = node->childNodes();
		for (const Core::AbstractDialogNode::Id& id : childNodes)
		{
			const auto childNodeIt = Core::findNodeById(nodes, id);
			if (childNodeIt == nodes.end())
			{
				nextPhaseItems.append(id);
			}
		}
	}

	const QList<PhaseGraphicsItem*> phasesList = m_nodesByPhase.keys();

	QList<PhaseGraphicsItem*> nextPhases;
	for (Core::AbstractDialogNode::Id nextPhaseItem : nextPhaseItems)
	{
		const auto it = std::find_if(phasesList.begin(), phasesList.end(),
			[nextPhaseItem, this](PhaseGraphicsItem* phaseItem)
			{
				const QList<Core::AbstractDialogNode*> phaseNodes = ::getPhaseNodes(m_nodesByPhase[phaseItem]);
				const auto it = Core::findNodeById(phaseNodes, nextPhaseItem);
				return it != phaseNodes.end();

			});

		if (it != phasesList.end())
		{
			nextPhases.append(*it);
		}
	}

	return nextPhases;
}

Core::PhaseNode DialogEditorWindow::getPhaseNode(PhaseGraphicsItem* phaseItem)
{
	Core::PhaseNode* phaseNode = phaseItem->data()->as<Core::PhaseNode>();

	QList<Core::AbstractDialogNode*> nodes;
	for (Core::AbstractDialogNode* node : phaseNode->nodes())
	{
		nodes.append(node->clone(false));
	}

	return Core::PhaseNode(*phaseNode);
}

QList<Core::AbstractDialogNode*> DialogEditorWindow::getPhaseNodes(PhaseGraphicsItem* phaseItem)
{
	QList<Core::AbstractDialogNode*> nodes;
	for (NodeGraphicsItem* nodeItem : m_nodesByPhase.value(phaseItem))
	{
		Core::AbstractDialogNode* node = nodeItem->data()->clone(false);
		nodes.append(node);
	}

	return nodes;
}
