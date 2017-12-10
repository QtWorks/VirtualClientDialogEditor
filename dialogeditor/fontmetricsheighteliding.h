#ifndef FONTMETRICSHEIGHTELIDING_H
#define FONTMETRICSHEIGHTELIDING_H

#include <QString>
#include <QFontMetrics>

QString elideText(const QFontMetrics& fontMetrics, const QString& text, int width, int height);

#endif // FONTMETRICSHEIGHTELIDING_H
