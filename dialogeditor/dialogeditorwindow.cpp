#include "dialogeditorwindow.h"
#include "ui_dialogeditorwindow.h"
#include "zoomablegraphicsview.h"
#include "dialogconstructorgraphicsscene.h"
#include "dialoggraphicsscene.h"
#include "core/dialogmodel.h"

#include "phasegraphicsitem.h"
#include "clientreplicanodegraphicsitem.h"
#include "expectedwordsnodegraphicsitem.h"
#include "arrowlinegraphicsitem.h"

#include "logger.h"
#include <QPushButton>

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

DialogEditorWindow::DialogEditorWindow(const Core::Dialog& dialog, QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::DialogEditorWindow)
	, m_dialogConstructorGraphicsScene(new DialogConstructorGraphicsScene(this))
	, m_dialogGraphicsScene(new DialogGraphicsScene(this))
	, m_dialogModel(make_unique<Core::DialogModel>(dialog))
{
	// TODO: set window title to dialog full name

	m_ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);

	m_ui->nameEdit->setText(dialog.name);
	connect(m_ui->nameEdit, &QLineEdit::textEdited,
		[this](const QString& name)
		{
			m_dialogModel->setName(name);
		});

	m_ui->difficultyComboBox->addItems(Core::Dialog::availableDifficulties());
	const int index = m_ui->difficultyComboBox->findText(Core::Dialog::difficultyToString(dialog.difficulty), Qt::MatchCaseSensitive);
	m_ui->difficultyComboBox->setCurrentIndex(index);
	connect(m_ui->difficultyComboBox, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
		[this](const QString& difficulty)
		{
			m_dialogModel->setDifficulty(Core::Dialog::difficultyFromString(difficulty));
		});

	QIcon warningIcon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
	QPixmap warningPixmap = warningIcon.pixmap(QSize(16, 16));
	m_ui->errorIconLabel->setPixmap(warningPixmap);

	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, [this]()
	{
		// TODO: emptiness validation, trims
		const QString name = m_dialogModel->name();
		const Core::Dialog::Difficulty difficulty = m_dialogModel->difficulty();

		emit dialogChanged({ name, difficulty, m_dialogModel->phases() });
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

	m_dialogGraphicsScene->setModel(m_dialogModel.get());

	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeSelectionChanged, this, &DialogEditorWindow::onNodeSelectionChanged);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeSelectionChanged, this, &DialogEditorWindow::updateConnectControls);

	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeAdded, this, &DialogEditorWindow::nodeAdded);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeRemoved, this, &DialogEditorWindow::nodeRemoved);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeChanged, this, &DialogEditorWindow::nodeChanged);

	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodesConnected, this, &DialogEditorWindow::nodesConnected);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodesDisconnected, this, &DialogEditorWindow::nodesDisconnected);

	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeAddedToPhase, this, &DialogEditorWindow::nodeAddedToPhase);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeRemovedFromPhase, this, &DialogEditorWindow::nodeRemovedFromPhase);

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

	LOG << ARG(m_selectedNodes.size());
}

void DialogEditorWindow::onLinkAdded(ArrowLineGraphicsItem* /*link*/)
{
}

void DialogEditorWindow::onLinkRemoved(ArrowLineGraphicsItem* /*link*/)
{
}

void DialogEditorWindow::updateSaveControls()
{
	QList<QGraphicsItem*> items = m_dialogGraphicsScene->items();

	if (items.isEmpty())
	{
		showError("Диалог не может быть пустым");
		return;
	}

	const int nodesWithoutIncomingLinks = std::count_if(items.begin(), items.end(),
		[](QGraphicsItem* node)
		{
			if (node->type() == ClientReplicaNodeGraphicsItem::Type ||
				node->type() == ExpectedWordsNodeGraphicsItem::Type)
			{
				NodeGraphicsItem* nodeItem = dynamic_cast<NodeGraphicsItem*>(node);
				return nodeItem->incomingLinks().isEmpty();
			}
			return false;
		});

	if (nodesWithoutIncomingLinks != 1)
	{
		showError("Должен быть только 1 узел без входящих стрелок (" + QString::number(nodesWithoutIncomingLinks) + ")");
		return;
	}

	const int nodesWithoutOutcomingLinks = std::count_if(items.begin(), items.end(),
		[](QGraphicsItem* node)
		{
			if (node->type() == ClientReplicaNodeGraphicsItem::Type ||
				node->type() == ExpectedWordsNodeGraphicsItem::Type)
			{
				NodeGraphicsItem* nodeItem = dynamic_cast<NodeGraphicsItem*>(node);
				return nodeItem->outcomingLinks().isEmpty();
			}
			return false;
		});

	if (nodesWithoutOutcomingLinks != 1)
	{
		showError("Должен быть только 1 узел без выходящих стрелок (" + QString::number(nodesWithoutOutcomingLinks) + ")");
		return;
	}

	hideError();
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

	NodeGraphicsItem* startItem = m_selectedNodes[0];
	NodeGraphicsItem* endItem = m_selectedNodes[1];

	m_dialogGraphicsScene->addLineToScene(new ArrowLineGraphicsItem(startItem, endItem, false));

	startItem->setSelected(false);
	endItem->setSelected(false);
}

void DialogEditorWindow::updateConnectControls()
{
	if (m_selectedNodes.size() != 2)
	{
		m_ui->connectNodesButton->setEnabled(false);
		return;
	}

	NodeGraphicsItem* startItem = m_selectedNodes[0];
	NodeGraphicsItem* endItem = m_selectedNodes[1];

	if (startItem->type() == endItem->type() && startItem->type() != PhaseGraphicsItem::Type)
	{
		m_ui->connectNodesButton->setEnabled(false);
		return;
	}

	if (m_dialogModel->difficulty() == Core::Dialog::Difficulty::Hard)
	{
		m_ui->connectNodesButton->setEnabled(true);
		return;
	}

	const bool nodesHasNoLinks = startItem->outcomingLinks().isEmpty() && endItem->incomingLinks().isEmpty();

	m_ui->connectNodesButton->setEnabled(nodesHasNoLinks);
}

void DialogEditorWindow::nodeAdded(NodeGraphicsItem* /*node*/, Core::AbstractDialogNode* /*nodeData*/)
{
	LOG;
}

void DialogEditorWindow::nodeRemoved(NodeGraphicsItem* /*node*/)
{
	LOG;
}

void DialogEditorWindow::nodeChanged(NodeGraphicsItem* /*node*/, Core::AbstractDialogNode* /*nodeData*/)
{
	LOG;
}

void DialogEditorWindow::nodesConnected(NodeGraphicsItem* /*parent*/, NodeGraphicsItem* /*child*/)
{
	LOG;
}

void DialogEditorWindow::nodesDisconnected(NodeGraphicsItem* /*parent*/, NodeGraphicsItem* /*child*/)
{
	LOG;
}

void DialogEditorWindow::nodeAddedToPhase(NodeGraphicsItem* /*node*/, PhaseGraphicsItem* /*phase*/)
{
	LOG;
}

void DialogEditorWindow::nodeRemovedFromPhase(NodeGraphicsItem* /*node*/, PhaseGraphicsItem* /*phase*/)
{
	LOG;
}
