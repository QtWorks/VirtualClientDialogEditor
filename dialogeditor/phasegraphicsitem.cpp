#include "phasegraphicsitem.h"
#include "phaseeditorwindow.h"

#include "clientreplicanodegraphicsitem.h"
#include "expectedwordsnodegraphicsitem.h"

#include <QKeyEvent>
#include <QGraphicsScene>

#include <QGraphicsSceneMouseEvent>

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

void PhaseGraphicsItem::doHack()
{
	QGraphicsSceneMouseEvent event;

	QRectF rect = mapRectToScene(boundingRect());
	QPointF eventPos = rect.bottomRight() - QPointF(1, 1);

	event.setScenePos(eventPos);

	LOG << "Set event pos " << eventPos;

	mousePressEvent(&event);

	mouseReleaseEvent(&event);
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
			item->setPosition(item->pos() + delta);
		}
	}
	else if (change == QGraphicsItem::ItemSelectedChange && scene())
	{
		const bool selected = value.toBool();
		for (NodeGraphicsItem* item : m_items)
		{
			item->setZValue(selected ? item->zValue() * 10 : item->zValue() / 10);
		}
	}

	return NodeGraphicsItem::itemChange(change, value);
}

QString PhaseGraphicsItem::getHeaderText() const
{
	return "Фаза: " + m_phase->name() + " (" + QString::number(m_phase->score()) + ")";
}

QString PhaseGraphicsItem::getContentText() const
{
	return QString();
}

QBrush PhaseGraphicsItem::getHeaderBrush() const
{
	return QColor::fromRgb(0xFFBC40);
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

template <typename T>
void replicateField(QVector<Core::ErrorReplica::Field> replicatingFields, Core::ErrorReplica::Field field,
	Core::ErrorReplica& oldPhaseError, Core::ErrorReplica& dialogError, const Core::ErrorReplica& newPhaseError)
{
	const bool replicateToAllPhases = std::find(replicatingFields.begin(), replicatingFields.end(), field) != replicatingFields.end();

	if (replicateToAllPhases)
	{
		dialogError.set(field, newPhaseError.get<T>(field));
		oldPhaseError.reset(field);
	}
	else
	{
		if (newPhaseError.get<T>(field) != dialogError.get<T>(field))
		{
			oldPhaseError.set(field, newPhaseError.get<T>(field));
		}
		else
		{
			oldPhaseError.reset(field);
		}
	}
}

void PhaseGraphicsItem::createEditorIfNeeded()
{
	if (m_editor)
	{
		return;
	}

	m_editor = new PhaseEditorWindow(*m_phase, *m_dialog);

	QObject::connect(m_editor, &PhaseEditorWindow::accepted, [this](const Core::PhaseNode& phase, QVector<Core::ErrorReplica::Field> replicatingFields)
	{
		m_phase->setName(phase.name());
		m_phase->setScore(phase.score());

		replicateField<QString>(replicatingFields, Core::ErrorReplica::Field::ErrorReplica,
			m_phase->errorReplica(), m_dialog->errorReplica, phase.errorReplica());
		replicateField<QList<QString>>(replicatingFields, Core::ErrorReplica::Field::FinishingExpectedWords,
			m_phase->errorReplica(), m_dialog->errorReplica, phase.errorReplica());
		replicateField<QString>(replicatingFields, Core::ErrorReplica::Field::FinishingReplica,
			m_phase->errorReplica(), m_dialog->errorReplica, phase.errorReplica());
		replicateField<QList<QString>>(replicatingFields, Core::ErrorReplica::Field::ContinuationExpectedWords,
			m_phase->errorReplica(), m_dialog->errorReplica, phase.errorReplica());

		update();

		emit changed();

		closeEditor();
	});

	QObject::connect(m_editor, &PhaseEditorWindow::rejected, [this]()
	{
		closeEditor();
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

void PhaseGraphicsItem::closeEditor()
{
	Q_ASSERT(m_editor);

	m_editor->close();
	m_editor = nullptr;
}
