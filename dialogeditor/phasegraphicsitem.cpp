#include "phasegraphicsitem.h"
#include "phaseeditorwindow.h"

#include "clientreplicanodegraphicsitem.h"
#include "expectedwordsnodegraphicsitem.h"

#include <QKeyEvent>
#include <QGraphicsScene>

PhaseGraphicsItem::PhaseGraphicsItem(Core::PhaseNode* phase, Properties properties, QObject* parent)
	: NodeGraphicsItem(properties, parent)
	, m_phase(phase)
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
	static const QColor s_color = QColor::fromRgb(65, 105, 225);
	return QBrush(s_color);
}

void PhaseGraphicsItem::showNodeEditor()
{
	createEditorIfNeeded();

	showEditor();
}

NodeGraphicsItem* PhaseGraphicsItem::clone() const
{
	return new PhaseGraphicsItem(m_phase->clone(true)->as<Core::PhaseNode>(), m_properties, parent());
}

qreal PhaseGraphicsItem::minHeight() const
{
	if (m_items.empty())
	{
		return NodeGraphicsItem::minHeight();
	}

	qreal mostBottom;
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

void PhaseGraphicsItem::createEditorIfNeeded()
{
	if (m_editor)
	{
		return;
	}

	m_editor = new PhaseEditorWindow(*m_phase);

	QObject::connect(m_editor, &PhaseEditorWindow::accepted, [this](const Core::PhaseNode& phase)
	{
		m_phase->setName(phase.name());
		m_phase->setScore(phase.score());

		update();

		closeEditor();
	});

	QObject::connect(m_editor, &PhaseEditorWindow::rejected, [this]()
	{
		closeEditor();
	});

	QObject::connect(m_editor, &QWidget::destroyed, [this]()
	{
		m_editor = nullptr;
	});

	QObject::connect(m_editor, &PhaseEditorWindow::changed, [this]()
	{
		update();
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
