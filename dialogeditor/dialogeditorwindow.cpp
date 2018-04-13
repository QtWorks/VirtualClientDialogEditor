#include "dialogeditorwindow.h"
#include "ui_dialogeditorwindow.h"
#include "zoomablegraphicsview.h"
#include "dialogconstructorgraphicsscene.h"
#include "dialoggraphicsscene.h"

#include "phasegraphicsitem.h"
#include "clientreplicanodegraphicsitem.h"
#include "expectedwordsnodegraphicsitem.h"

#include "logger.h"
#include <QPushButton>

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

DialogEditorWindow::DialogEditorWindow(const Core::Dialog& dialog, QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::DialogEditorWindow)
	, m_dialogConstructorGraphicsScene(new DialogConstructorGraphicsScene(this))
	, m_dialogGraphicsScene(new DialogGraphicsScene(this))
	, m_dialog(dialog)
{
	m_ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose, true);

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
		});

	QIcon warningIcon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
	QPixmap warningPixmap = warningIcon.pixmap(QSize(16, 16));
	m_ui->errorIconLabel->setPixmap(warningPixmap);

	m_ui->buttonBox->button(QDialogButtonBox::Save)->setText("Сохранить");
	m_ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отменить");
	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, [this]()
	{
		Q_ASSERT(validateDialog());

		updateDialog();

		emit dialogChanged(m_dialog);

		close();
	});

	connect(m_ui->buttonBox, &QDialogButtonBox::rejected, [this]()
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

	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeSelectionChanged, this, &DialogEditorWindow::onNodeSelectionChanged);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeSelectionChanged, this, &DialogEditorWindow::updateConnectControls);

	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeAdded, this, &DialogEditorWindow::nodeAdded);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeAdded, this, &DialogEditorWindow::updateSaveControls);

	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeRemoved, this, &DialogEditorWindow::nodeRemoved);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeRemoved, this, &DialogEditorWindow::updateSaveControls);

	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeChanged, this, &DialogEditorWindow::nodeChanged);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeChanged, this, &DialogEditorWindow::updateSaveControls);

	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeAddedToPhase, this, &DialogEditorWindow::nodeAddedToPhase);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeAddedToPhase, this, &DialogEditorWindow::updateSaveControls);

	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeRemovedFromPhase, this, &DialogEditorWindow::nodeRemovedFromPhase);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeRemovedFromPhase, this, &DialogEditorWindow::updateSaveControls);

	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodesConnected, this, &DialogEditorWindow::nodesConnected);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodesConnected, this, &DialogEditorWindow::updateSaveControls);

	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodesDisconnected, this, &DialogEditorWindow::nodesDisconnected);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodesDisconnected, this, &DialogEditorWindow::updateSaveControls);

	m_dialogGraphicsScene->setDialog(&m_dialog);

	m_ui->dialogGraphicsView->setScene(m_dialogGraphicsScene);
	m_ui->dialogGraphicsView->setMinRatio(50.0);
	m_ui->dialogGraphicsView->setMaxRatio(150.0);

	updateSaveControls();
	updateConnectControls();
}

DialogEditorWindow::~DialogEditorWindow()
{
	delete m_ui;
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

	m_ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
}

void DialogEditorWindow::hideError()
{
	m_ui->errorIconLabel->hide();

	m_ui->errorTextLabel->setText("");
	m_ui->errorTextLabel->hide();

	m_ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
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

void DialogEditorWindow::updateConnectControls()
{
	if (m_selectedNodes.size() != 2)
	{
		m_ui->connectNodesButton->setEnabled(false);
		return;
	}

	NodeGraphicsItem* parentNode = m_selectedNodes[0];
	NodeGraphicsItem* childNode = m_selectedNodes[1];

	if (parentNode->type() == childNode->type() && parentNode->type() != PhaseGraphicsItem::Type)
	{
		m_ui->connectNodesButton->setEnabled(false);
		return;
	}

	if (m_dialog.difficulty == Core::Dialog::Difficulty::Hard)
	{
		m_ui->connectNodesButton->setEnabled(true);
		return;
	}

	const bool nodesHasNoLinks = parentNode->outcomingLinks().isEmpty() && childNode->incomingLinks().isEmpty();

	m_ui->connectNodesButton->setEnabled(nodesHasNoLinks);
}

void DialogEditorWindow::nodeAdded(NodeGraphicsItem* node)
{
	LOG << ARG2(nodeType(node), "node");

	Q_ASSERT(!m_nodeItems.contains(node));
	m_nodeItems.push_back(node);

	if (node->type() == PhaseGraphicsItem::Type)
	{
		m_nodesByPhase.insert(qgraphicsitem_cast<PhaseGraphicsItem*>(node), {});
	}
}

void DialogEditorWindow::nodeRemoved(NodeGraphicsItem* node)
{
	LOG << ARG2(nodeType(node), "node");

	Q_ASSERT(m_nodeItems.contains(node));
	m_nodeItems.removeOne(node);

	if (node->type() == PhaseGraphicsItem::Type)
	{
		m_nodesByPhase.remove(qgraphicsitem_cast<PhaseGraphicsItem*>(node));
	}

	if (m_selectedNodes.contains(node))
	{
		node->setSelected(false);
	}
}

void DialogEditorWindow::nodeChanged(NodeGraphicsItem* originalNode, NodeGraphicsItem* updatedNode)
{
	LOG << ARG2(nodeType(originalNode), "originalNode") << ARG2(nodeType(updatedNode), "updatedNode");
}

void DialogEditorWindow::nodesConnected(NodeGraphicsItem* parent, NodeGraphicsItem* child)
{
	LOG << ARG2(nodeType(parent), "parent") << ARG2(nodeType(child), "child");

	Q_ASSERT(m_nodeItems.contains(parent));
	Q_ASSERT(m_nodeItems.contains(child));

	Core::AbstractDialogNode* parentNode = parent->data();
	Core::AbstractDialogNode* childNode = child->data();

	parentNode->appendChild(childNode);
	childNode->appendParent(parentNode);
}

void DialogEditorWindow::nodesDisconnected(NodeGraphicsItem* parent, NodeGraphicsItem* child)
{
	LOG << ARG2(nodeType(parent), "parent") << ARG2(nodeType(child), "child");

	Q_ASSERT(m_nodeItems.contains(parent));
	Q_ASSERT(m_nodeItems.contains(child));

	Core::AbstractDialogNode* parentNode = parent->data();
	Core::AbstractDialogNode* childNode = child->data();

	Q_ASSERT(parentNode->childNodes().contains(childNode));
	parentNode->removeChild(childNode);
	childNode->removeParent(parentNode);
}

void DialogEditorWindow::nodeAddedToPhase(NodeGraphicsItem* node, PhaseGraphicsItem* phase)
{
	LOG << ARG2(nodeType(node), "node");
	Q_ASSERT(m_nodesByPhase.contains(phase));

	m_nodesByPhase[phase].append(node);
}

void DialogEditorWindow::nodeRemovedFromPhase(NodeGraphicsItem* node, PhaseGraphicsItem* phase)
{
	LOG << ARG2(nodeType(node), "node");
	Q_ASSERT(m_nodesByPhase.contains(phase));

	m_nodesByPhase[phase].removeOne(node);
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

	return true;
}

void DialogEditorWindow::updateDialog()
{
	Q_ASSERT(validateDialog());

	for (PhaseGraphicsItem* phase : m_nodesByPhase.keys())
	{
		QList<NodeGraphicsItem*> nodes = m_nodesByPhase.value(phase);
		std::sort(nodes.begin(), nodes.end(),
			[](NodeGraphicsItem* left, NodeGraphicsItem* right)
			{
				return left->data()->parentNodes().empty() || left->data()->childNodes().contains(right->data());
			});
	}

	const auto firstPhaseIt = std::find_if(m_nodesByPhase.keyBegin(), m_nodesByPhase.keyEnd(),
		[this](PhaseGraphicsItem* phase)
		{
			return m_nodesByPhase.value(phase).first()->data()->parentNodes().empty();
		});
	Q_ASSERT(firstPhaseIt != m_nodesByPhase.keyEnd());

	QList<PhaseGraphicsItem*> phaseItems;
	phaseItems.append(*firstPhaseIt);

	while (phaseItems.size() != m_nodesByPhase.keys().size())
	{
		NodeGraphicsItem* previousPhaseEnd = m_nodesByPhase[phaseItems.last()].last();
		Core::AbstractDialogNode* nextPhaseBegin = *previousPhaseEnd->data()->childNodes().begin();

		const auto nextPhaseIt = std::find_if(m_nodesByPhase.keyBegin(), m_nodesByPhase.keyEnd(),
			[this, nextPhaseBegin](PhaseGraphicsItem* phase)
			{
			  return m_nodesByPhase.value(phase).first()->data() == nextPhaseBegin;
			});
		Q_ASSERT(nextPhaseIt != m_nodesByPhase.keyEnd());

		phaseItems.append(*nextPhaseIt);
	}

	QList<Core::PhaseNode> phaseNodes;
	for (PhaseGraphicsItem* phaseItem : phaseItems)
	{
		Core::PhaseNode* phase = dynamic_cast<Core::PhaseNode*>(phaseItem->data());

		const QList<NodeGraphicsItem*>& phaseNodeItems = m_nodesByPhase.value(phaseItem);

		Core::AbstractDialogNode* root = m_nodesByPhase.value(phaseItem).first()->data()->shallowCopy();
		Core::AbstractDialogNode* parent = root;
		Core::AbstractDialogNode* child = nullptr;

		for (int i = 1; i < phaseNodeItems.size(); ++i)
		{
			child = phaseNodeItems[i]->data()->shallowCopy();

			parent->appendChild(child);
			child->appendParent(parent);

			parent = child;
		}

		phaseNodes.append(Core::PhaseNode(phase->name, phase->score, root));
	}

	m_dialog.phases = phaseNodes;
}
