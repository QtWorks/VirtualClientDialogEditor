#include "fontmetricsheighteliding.h"

#include <QHash>

namespace
{

const QChar c_newLine = '\n';

int calculateCharacterWidth(QChar symbol, const QFontMetrics& fontMetrics)
{
	static QHash<QChar, int> s_cache;
	const auto it = s_cache.find(symbol);
	if (it != s_cache.end())
	{
		return *it;
	}

	auto insertedIt = s_cache.insert(symbol, fontMetrics.width(symbol));
	return *insertedIt;
}

template <typename InputIterator>
QString textForWidth(InputIterator& first, InputIterator last, int maxWidth, const QFontMetrics& fontMetrics)
{
	QString result;
	int resultWidth = 0;

	while (first != last)
	{
		const QChar& symbol = *first;

		if (symbol == c_newLine)
		{
			++first;
			break;
		}

		if (resultWidth >= maxWidth)
		{
			break;
		}

		int symbolWidth = calculateCharacterWidth(symbol, fontMetrics);
		if (resultWidth + symbolWidth > maxWidth)
		{
			break;
		}

		resultWidth += symbolWidth;
		result += symbol;
		++first;
	}

	return result;
}

}

QString elideText(const QFontMetrics& fontMetrics, const QString& text, int width, int height)
{
	QString result;

	const int contentLinesAvailable = height / fontMetrics.height();

	auto textIterator = text.begin();
	for (int i = 0; i < contentLinesAvailable && textIterator != text.end(); ++i)
	{
		QString lineText = textForWidth(textIterator, text.end(), width, fontMetrics);
		if (i == contentLinesAvailable - 1 && textIterator != text.end()) // last line and there is more text
		{
			int lineTextWidth = fontMetrics.width(lineText);
			int nextSymbolWidth = calculateCharacterWidth(*textIterator, fontMetrics);

			if (lineTextWidth + nextSymbolWidth > width)
			{
				lineText = fontMetrics.elidedText(lineText, Qt::ElideRight, lineTextWidth - 1);
			}
		}

		if (!result.isEmpty())
		{
			result += c_newLine;
		}

		result += lineText;
	}

	return result;
}
