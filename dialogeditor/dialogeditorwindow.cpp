#include "dialogeditorwindow.h"
#include "ui_dialogeditorwindow.h"
#include "dialogconstructorgraphicsscene.h"
#include "dialoggraphicsscene.h"
#include "core/dialogmodel.h"

#include <QPushButton>
#include "clientreplicanodegraphicsitem.h"
#include "expectedwordsnodegraphicsitem.h"

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

DialogEditorWindow::DialogEditorWindow(const Dialog& dialog, QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::DialogEditorWindow)
	, m_dialogConstructorGraphicsScene(new DialogConstructorGraphicsScene(this))
	, m_dialogGraphicsScene(new DialogGraphicsScene(this))
	, m_dialogModel(make_unique<DialogModel>(dialog))
{
	// TODO: set window title to dialog full name

	m_ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);

	m_ui->nameEdit->setText(dialog.name);

	m_ui->difficultyComboBox->addItems(Dialog::availableDifficulties());
	const int index = m_ui->difficultyComboBox->findText(Dialog::difficultyToString(dialog.difficulty), Qt::MatchCaseSensitive);
	m_ui->difficultyComboBox->setCurrentIndex(index);

	QIcon warningIcon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
	QPixmap warningPixmap = warningIcon.pixmap(QSize(16, 16));
	m_ui->errorIconLabel->setPixmap(warningPixmap);

	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, [this]()
	{
		// TODO: emptiness validation, trims
		const QString username = m_ui->nameEdit->text();
		const Dialog::Difficulty difficulty = Dialog::difficultyFromString(m_ui->difficultyComboBox->currentText());

		// TODO: collect that shit together - nodes, transitions, etc.

		emit dialogChanged({ username, difficulty, { }});
		//accept();
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

	m_dialogGraphicsScene->setModel(m_dialogModel.get());
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeAdded, this, &DialogEditorWindow::updateControls);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::nodeRemoved, this, &DialogEditorWindow::updateControls);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::linkAdded, this, &DialogEditorWindow::updateControls);
	connect(m_dialogGraphicsScene, &DialogGraphicsScene::linkRemoved, this, &DialogEditorWindow::updateControls);
	// TODO: itemsLinked

	m_ui->dialogGraphicsView->setScene(m_dialogGraphicsScene);

	updateControls();
}

DialogEditorWindow::~DialogEditorWindow()
{
	delete m_ui;
}

void DialogEditorWindow::updateControls()
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
				LOG << ARG2(nodeItem->type(), "type") << ARG2(nodeItem->incomingLinks().size(), "incomingLinks");
				return nodeItem->incomingLinks().isEmpty();
			}
			return false;
		});

	if (nodesWithoutIncomingLinks != 1)
	{
		// TODO: message
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
		// TODO: message
		showError("Должен быть только 1 узел без выходящих стрелок (" + QString::number(nodesWithoutOutcomingLinks) + ")");
		return;
	}

	hideError();
}

void DialogEditorWindow::showError(QString text)
{
	m_ui->errorIconLabel->setVisible(true);
	m_ui->errorTextLabel->setText(text);
	m_ui->errorTextLabel->setVisible(true);
	m_ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
}

void DialogEditorWindow::hideError()
{
	m_ui->errorIconLabel->setVisible(false);
	m_ui->errorTextLabel->setText("");
	m_ui->errorTextLabel->setVisible(false);
	m_ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
}
