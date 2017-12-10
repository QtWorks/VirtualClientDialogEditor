#include "clientreplicanodegraphicsitem.h"
#include "clientreplicaeditor.h"

ClientReplicaNodeGraphicsItem::ClientReplicaNodeGraphicsItem(const QString& replica, Properties properties, QObject* parent)
	: NodeGraphicsItem(properties, parent)
	, m_replica(replica)
	, m_editor(nullptr)
{
}

int ClientReplicaNodeGraphicsItem::type() const
{
	return Type;
}

QString ClientReplicaNodeGraphicsItem::getText() const
{
	QString result = "Клиент";
	if (!m_replica.isEmpty())
	{
		result += ":\n" + m_replica;
	}
	return result;
}

QBrush ClientReplicaNodeGraphicsItem::getBrush() const
{
	//FF4081
	//static const QColor s_lightRed = QColor::fromRgb(0xF4, 0x43, 0x36);
	static const QColor s_lightRed = QColor::fromRgb(0xFF, 0x52, 0x52);
	return QBrush(s_lightRed);
}

void ClientReplicaNodeGraphicsItem::showNodeEditor()
{
	createEditorIfNeeded();

	showEditor();
}

NodeGraphicsItem* ClientReplicaNodeGraphicsItem::clone() const
{
	return new ClientReplicaNodeGraphicsItem(m_replica, m_properties);
}

void ClientReplicaNodeGraphicsItem::createEditorIfNeeded()
{
	if (m_editor)
	{
		return;
	}

	m_editor = new ClientReplicaEditor(m_replica);

	QObject::connect(m_editor, &ClientReplicaEditor::accepted, [this](QString replica)
	{
		m_replica = replica;

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
