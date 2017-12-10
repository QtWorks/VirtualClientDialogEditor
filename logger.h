#ifndef LOGGER_H
#define LOGGER_H

#include <QDebug>
#include <QDateTime>

#define ARG2(value, name) (" " name "=") << (value)
#define ARG(value) ARG2(value, #value)

inline QString methodName(const std::string& prettyFunction)
{
	size_t colons = prettyFunction.find("::");
	size_t begin = prettyFunction.substr(0,colons).rfind(" ") + 1;
	size_t end = prettyFunction.rfind("(") - begin;

	return QString::fromStdString(prettyFunction.substr(begin,end) + "()");
}

#define __METHOD_NAME__ methodName(__PRETTY_FUNCTION__)

#define LOG qDebug().noquote().nospace() << QDateTime::currentDateTime().time() << " " << __METHOD_NAME__ << " "

#endif // LOGGER_H
