#include "phasegraphicsitem.h"
#include "phaseeditorwindow.h"

#include "clientreplicanodegraphicsitem.h"
#include "expectedwordsnodegraphicsitem.h"

#include <QKeyEvent>
#include <QGraphicsScene>

#include <QGraphicsSceneMouseEvent>

namespace {

const int c_zValueMultiplier = 10;

}

PhaseGraphicsItem::PhaseGraphicsItem(Core::PhaseNode* phase, Core::Dialog* dialog, Properties properties, QObject* parent)
	: NodeGraphicsItem(properties, parent)
	, m_phase(phase)
	, m_dialog(dialog)
	, m_editor(nullptr)
{
	setZValue(1.0);
}

void PhaseGraphicsItem::addItem(NodeGraphicsItem* item)
{
	if (item == this)
	{
		return;
	}

	if (!m_items.contains(item))
	{
		if (isSelected())
		{
			item->setZValue(item->zValue() * c_zValueMultiplier);
		}

		m_items.append(item);
		item->setPhase(this);
	}
}

void PhaseGraphicsItem::removeItem(NodeGraphicsItem* item)
{
	if (item == this)
	{
		return;
	}

	m_items.removeOne(item);
	item->setPhase(nullptr);
}

QList<NodeGraphicsItem*> PhaseGraphicsItem::items()
{
	return m_items;
}

void PhaseGraphicsItem::setDialog(Core::Dialog* dialog)
{
	m_dialog = dialog;
}

int PhaseGraphicsItem::type() const
{
	return PhaseGraphicsItem::Type;
}

Core::AbstractDialogNode* PhaseGraphicsItem::data()
{
	return m_phase;
}

const Core::AbstractDialogNode* PhaseGraphicsItem::data() const
{
	return m_phase;
}

void PhaseGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	if (event->pos().y() < m_headerHeight)
	{
		setToolTip(getHeaderText());
	}
	else
	{
		setToolTip("");
	}

	NodeGraphicsItem::hoverEnterEvent(event);
}

void PhaseGraphicsItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
	if (event->pos().y() < m_headerHeight)
	{
		setToolTip(getHeaderText());
	}
	else
	{
		setToolTip("");
	}

	NodeGraphicsItem::hoverMoveEvent(event);
}

void PhaseGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	setToolTip("");

	NodeGraphicsItem::hoverLeaveEvent(event);
}

void PhaseGraphicsItem::keyPressEvent(QKeyEvent* event)
{
	if (m_properties & Removable && event->key() == Qt::Key_Delete)
	{
		for (NodeGraphicsItem* item : m_items)
		{
			item->keyPressEvent(event);
		}
	}

	NodeGraphicsItem::keyPressEvent(event);
}

QVariant PhaseGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
	if (change == QGraphicsItem::ItemPositionChange && scene())
	{
		QPointF delta = value.toPointF() - pos();
		for (NodeGraphicsItem* item : m_items)
		{
			item->setPos(item->pos() + delta);
		}
	}
	else if (change == QGraphicsItem::ItemSelectedChange && scene())
	{
		const bool selected = value.toBool();
		for (NodeGraphicsItem* item : m_items)
		{
			item->setZValue(selected ? item->zValue() * c_zValueMultiplier : item->zValue() / c_zValueMultiplier);
		}
	}

	return NodeGraphicsItem::itemChange(change, value);
}

QString PhaseGraphicsItem::getHeaderText() const
{
	return QString("Фаза: %1 (%2 / %3)").arg(m_phase->name()).arg(m_phase->bestPossibleScore()).arg(m_phase->score());
}

QColor PhaseGraphicsItem::getHeaderTextColor() const
{
	return Qt::black;
}

QColor PhaseGraphicsItem::getHeaderBackgroundColor() const
{
	return QColor::fromRgb(0xFFBC40);
}

QString PhaseGraphicsItem::getContentText() const
{
	return QString();
}

QColor PhaseGraphicsItem::getContentTextColor() const
{
	return Qt::black;
}

QColor PhaseGraphicsItem::getContentBackgroundColor() const
{
	return Qt::white;
}

void PhaseGraphicsItem::showNodeEditor()
{
	createEditorIfNeeded();

	showEditor();
}

NodeGraphicsItem* PhaseGraphicsItem::clone() const
{
	return new PhaseGraphicsItem(m_phase->clone(true)->as<Core::PhaseNode>(), m_dialog, m_properties, parent());
}

qreal PhaseGraphicsItem::minHeight() const
{
	if (m_items.empty())
	{
		return NodeGraphicsItem::minHeight();
	}

	qreal mostBottom = 0.0;
	for (NodeGraphicsItem* item : m_items)
	{
		qreal bottom = item->sceneBoundingRect().bottom() - scenePos().y();
		if (bottom > mostBottom)
		{
			mostBottom = bottom;
		}
	}

	return mostBottom + 15.0;
}

qreal PhaseGraphicsItem::minWidth() const
{
	if (m_items.empty())
	{
		return NodeGraphicsItem::minWidth() + 30.0;
	}

	qreal mostRight = 0.0;
	for (NodeGraphicsItem* item : m_items)
	{
		qreal right = item->sceneBoundingRect().right() - scenePos().x();
		if (right > mostRight)
		{
			mostRight = right;
		}
	}

	return mostRight + 15.0;
}

void PhaseGraphicsItem::onPhaseAccepted(const Core::PhaseNode& phase, const Core::ErrorReplica& globalErrorReplica, const Optional<QString>& globalRepeatReplica)
{
	m_phase->setName(phase.name());
	m_phase->setScore(phase.score());
	m_phase->setRepeatOnInsufficientScore(phase.repeatOnInsufficientScore());

	if (globalErrorReplica.errorReplica)
	{
		m_dialog->errorReplica.errorReplica = *globalErrorReplica.errorReplica;

		for (Core::PhaseNode& phase : m_dialog->phases)
		{
			phase.errorReplica().errorReplica = Optional<QString>();
		}
		m_phase->errorReplica().errorReplica = Optional<QString>();
	}
	else
	{
		m_phase->errorReplica().errorReplica = *phase.errorReplica().errorReplica;
	}

	if (globalErrorReplica.errorPenalty)
	{
		m_dialog->errorReplica.errorPenalty = *globalErrorReplica.errorPenalty;

		for (Core::PhaseNode& phase : m_dialog->phases)
		{
			phase.errorReplica().errorPenalty = Optional<double>();
		}
		m_phase->errorReplica().errorPenalty = Optional<double>();
	}
	else
	{
		m_phase->errorReplica().errorPenalty = *phase.errorReplica().errorPenalty;
	}

	if (globalErrorReplica.finishingExpectedWords)
	{
		m_dialog->errorReplica.finishingExpectedWords = *globalErrorReplica.finishingExpectedWords;

		for (Core::PhaseNode& phase : m_dialog->phases)
		{
			phase.errorReplica().finishingExpectedWords = Optional<QList<QString>>();
		}
		m_phase->errorReplica().finishingExpectedWords = Optional<QList<QString>>();
	}
	else
	{
		m_phase->errorReplica().finishingExpectedWords = *phase.errorReplica().finishingExpectedWords;
	}

	if (globalErrorReplica.finishingReplica)
	{
		m_dialog->errorReplica.finishingReplica = *globalErrorReplica.finishingReplica;

		for (Core::PhaseNode& phase : m_dialog->phases)
		{
			phase.errorReplica().finishingReplica = Optional<QString>();
		}
		m_phase->errorReplica().finishingReplica = Optional<QString>();
	}
	else
	{
		m_phase->errorReplica().finishingReplica = *phase.errorReplica().finishingReplica;
	}

	if (globalRepeatReplica)
	{
		m_dialog->phaseRepeatReplica = *globalRepeatReplica;

		for (Core::PhaseNode& phase : m_dialog->phases)
		{
			phase.repeatReplica() = Optional<QString>();
		}
	}
	else
	{
		m_phase->repeatReplica() = *phase.repeatReplica();
	}

	update();

	emit changed();

	closeEditor(true);
}

void PhaseGraphicsItem::createEditorIfNeeded()
{
	if (m_editor)
	{
		return;
	}

	m_editor = new PhaseEditorWindow(*m_phase, *m_dialog);

	QObject::connect(m_editor, &PhaseEditorWindow::accepted, this, &PhaseGraphicsItem::onPhaseAccepted);

	QObject::connect(m_editor, &PhaseEditorWindow::rejected, [this]()
	{
		closeEditor(false);
	});

	QObject::connect(m_editor, &PhaseEditorWindow::changed, [this]()
	{
		update();
	});

	QObject::connect(m_editor, &QWidget::destroyed, [this]()
	{
		m_editor = nullptr;
	});
}

void PhaseGraphicsItem::showEditor()
{
	m_editor->setWindowState(m_editor->windowState() & (~Qt::WindowMinimized | Qt::WindowActive));
	m_editor->activateWindow();
	m_editor->show();
}

void PhaseGraphicsItem::closeEditor(bool accepted)
{
	Q_ASSERT(m_editor);

	emit editorClosed(accepted);

	m_editor->close();
	m_editor = nullptr;
}
