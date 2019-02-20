#include "expectedwordsnodegraphicsitem.h"
#include "expectedwordseditorwindow.h"

ExpectedWordsNodeGraphicsItem::ExpectedWordsNodeGraphicsItem(Core::ExpectedWordsNode* expectedWords, Properties properties, QObject* parent)
	: NodeGraphicsItem(properties, parent)
	, m_expectedWords(expectedWords)
	, m_editor(nullptr)
{
	setZValue(3.0);
}

int ExpectedWordsNodeGraphicsItem::type() const
{
	return Type;
}

Core::AbstractDialogNode* ExpectedWordsNodeGraphicsItem::data()
{
	return m_expectedWords;
}

const Core::AbstractDialogNode* ExpectedWordsNodeGraphicsItem::data() const
{
	return m_expectedWords;
}

QString ExpectedWordsNodeGraphicsItem::getHeaderText() const
{
	return "Опорные слова";
}

QColor ExpectedWordsNodeGraphicsItem::getHeaderTextColor() const
{
	return m_expectedWords->forbidden() ? Qt::white : Qt::black;
}

QColor ExpectedWordsNodeGraphicsItem::getHeaderBackgroundColor() const
{
	return m_expectedWords->forbidden() ? Qt::black : QColor::fromRgb(0x5DCFC3);
}

QString ExpectedWordsNodeGraphicsItem::getContentText() const
{
	QStringList prefexedExpectedWords;

	for (int i = 0; i < m_expectedWords->expectedWords().size(); ++i)
	{
		prefexedExpectedWords << "- " + m_expectedWords->expectedWords()[i].words + (i == m_expectedWords->expectedWords().size() - 1 ? "." : ";");
	}

	return prefexedExpectedWords.join("\n");
}

QColor ExpectedWordsNodeGraphicsItem::getContentTextColor() const
{
	return Qt::black;
}

QColor ExpectedWordsNodeGraphicsItem::getContentBackgroundColor() const
{
	return Qt::white;
}

void ExpectedWordsNodeGraphicsItem::showNodeEditor()
{
	createEditorIfNeeded();

	showEditor();
}

NodeGraphicsItem* ExpectedWordsNodeGraphicsItem::clone() const
{
	return new ExpectedWordsNodeGraphicsItem(m_expectedWords->clone(true)->as<Core::ExpectedWordsNode>(), m_properties, parent());
}

void ExpectedWordsNodeGraphicsItem::createEditorIfNeeded()
{
	if (m_editor)
	{
		return;
	}

	m_editor = new ExpectedWordsEditorWindow(*m_expectedWords);

	QObject::connect(m_editor, &ExpectedWordsEditorWindow::accepted, [this](const Core::ExpectedWordsNode& expectedWords)
	{
		m_expectedWords->setExpectedWords(expectedWords.expectedWords());

		if (!m_expectedWords->forbidden())
		{
			m_expectedWords->setMinScore(expectedWords.minScore());
		}

		if (expectedWords.customHint())
		{
			m_expectedWords->setCustomHint(true);
			m_expectedWords->setHint(expectedWords.hint());
		}
		else
		{
			m_expectedWords->setCustomHint(false);
		}

		update();

		emit changed();

		closeEditor();
	});

	QObject::connect(m_editor, &ExpectedWordsEditorWindow::rejected, [this]()
	{
		closeEditor();
	});

	QObject::connect(m_editor, &QWidget::destroyed, [this]()
	{
		m_editor = nullptr;
	});
}

void ExpectedWordsNodeGraphicsItem::showEditor()
{
	m_editor->setWindowState(m_editor->windowState() & (~Qt::WindowMinimized | Qt::WindowActive));
	m_editor->activateWindow();
	m_editor->show();
}

void ExpectedWordsNodeGraphicsItem::closeEditor()
{
	Q_ASSERT(m_editor);

	m_editor->close();
	m_editor = nullptr;
}
