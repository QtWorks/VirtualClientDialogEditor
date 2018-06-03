#include "clientreplicanodegraphicsitem.h"
#include "clientreplicaeditor.h"

ClientReplicaNodeGraphicsItem::ClientReplicaNodeGraphicsItem(Core::ClientReplicaNode* replica, Properties properties, QObject* parent)
	: NodeGraphicsItem(properties, parent)
	, m_replica(replica)
	, m_editor(nullptr)
{
	setZValue(3.0);
}

int ClientReplicaNodeGraphicsItem::type() const
{
	return Type;
}

Core::AbstractDialogNode* ClientReplicaNodeGraphicsItem::data()
{
	return m_replica;
}

const Core::AbstractDialogNode* ClientReplicaNodeGraphicsItem::data() const
{
	return m_replica;
}

QString ClientReplicaNodeGraphicsItem::getHeaderText() const
{
	return "Клиент";
}

QString ClientReplicaNodeGraphicsItem::getContentText() const
{
	return m_replica->replica();
}

QBrush ClientReplicaNodeGraphicsItem::getHeaderBrush() const
{
	return QBrush(QColor::fromRgb(0xFF, 0x52, 0x52));
}

void ClientReplicaNodeGraphicsItem::showNodeEditor()
{
	createEditorIfNeeded();

	showEditor();
}

NodeGraphicsItem* ClientReplicaNodeGraphicsItem::clone() const
{
	return new ClientReplicaNodeGraphicsItem(m_replica->clone(true)->as<Core::ClientReplicaNode>(), m_properties, parent());
}

void ClientReplicaNodeGraphicsItem::createEditorIfNeeded()
{
	if (m_editor)
	{
		return;
	}

	m_editor = new ClientReplicaEditor(*m_replica);

	QObject::connect(m_editor, &ClientReplicaEditor::accepted, [this](const Core::ClientReplicaNode& replica)
	{
		m_replica->setReplica(replica.replica());

		update();

		closeEditor();
	});

	QObject::connect(m_editor, &ClientReplicaEditor::rejected, [this]()
	{
		closeEditor();
	});

	QObject::connect(m_editor, &QWidget::destroyed, [this]()
	{
		m_editor = nullptr;
	});
}

void ClientReplicaNodeGraphicsItem::showEditor()
{
	m_editor->setWindowState(m_editor->windowState() & (~Qt::WindowMinimized | Qt::WindowActive));
	m_editor->activateWindow();
	m_editor->show();
}

void ClientReplicaNodeGraphicsItem::closeEditor()
{
	Q_ASSERT(m_editor);

	m_editor->close();
	m_editor = nullptr;
}
