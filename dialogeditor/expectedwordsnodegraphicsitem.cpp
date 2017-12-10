#include "expectedwordsnodegraphicsitem.h"
#include "expectedwordseditorwindow.h"

ExpectedWordsNodeGraphicsItem::ExpectedWordsNodeGraphicsItem(const QStringList& expectedWords, Properties properties, QObject* parent)
	: NodeGraphicsItem(properties, parent)
	, m_expectedWords(expectedWords)
	, m_editor(nullptr)
{
}

int ExpectedWordsNodeGraphicsItem::type() const
{
	return Type;
}

QString ExpectedWordsNodeGraphicsItem::getText() const
{
	QString result = "Опорные слова";

	if (m_expectedWords.isEmpty())
	{
		return result;
	}

	QStringList prefexedExpectedWords;

	for (int i = 0; i < m_expectedWords.size(); ++i)
	{
		prefexedExpectedWords << "- " + m_expectedWords[i] + (i == m_expectedWords.size() - 1 ? "." : ";");
	}

	result += ":\n" + prefexedExpectedWords.join("\n");
	return result;
}

QBrush ExpectedWordsNodeGraphicsItem::getBrush() const
{
	//static const QColor s_lightGreen = QColor::fromRgb(0x4C, 0xAF, 0x50);
	//4FC3F7
	//81D4FA
	static const QColor s_lightGreen = QColor::fromRgb(0x4F, 0xC3, 0xF7);
	return QBrush(s_lightGreen);
}

void ExpectedWordsNodeGraphicsItem::showNodeEditor()
{
	createEditorIfNeeded();

	showEditor();
}

NodeGraphicsItem* ExpectedWordsNodeGraphicsItem::clone() const
{
	return new ExpectedWordsNodeGraphicsItem(m_expectedWords, m_properties);
}

void ExpectedWordsNodeGraphicsItem::createEditorIfNeeded()
{
	if (m_editor)
	{
		return;
	}

	m_editor = new ExpectedWordsEditorWindow(m_expectedWords);

	QObject::connect(m_editor, &ExpectedWordsEditorWindow::accepted, [this](QStringList expectedWords)
	{
		m_expectedWords = expectedWords;

		update();

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
