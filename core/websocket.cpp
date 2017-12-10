#include "websocket.h"

WebSocket::WebSocket(QObject* parent)
	: QObject(parent)
	, m_webSocket(new QWebSocket("ws://vcappdemo.herokuapp.com/", QWebSocketProtocol::VersionLatest, parent))
{
	/*QObject::connect(m_webSocket, &QWebSocket::aboutToClose, []()
	{
		qDebug() << "aboutToClose";
	});
	QObject::connect(m_webSocket, &QWebSocket::connected, []()
	{
		qDebug() << "connected";
	});
	QObject::connect(m_webSocket, &QWebSocket::disconnected, []()
	{
		qDebug() << "disconnected";
	});
	QObject::connect(m_webSocket, &QWebSocket::stateChanged, [](QAbstractSocket::SocketState state)
	{
		qDebug() << "stateChanged " << state;
	});
	QObject::connect(m_webSocket, &QWebSocket::readChannelFinished, []()
	{
		qDebug() << "readChannelFinished";
	});
	QObject::connect(m_webSocket, &QWebSocket::textFrameReceived, [](const QString& frame, bool isLastFrame)
	{
		qDebug() << "textFrameReceived " << frame << isLastFrame;
	});
	QObject::connect(m_webSocket, &QWebSocket::binaryFrameReceived, [](const QByteArray& frame, bool isLastFrame)
	{
		qDebug() << "binaryFrameReceived " << frame << isLastFrame;
	});
	QObject::connect(m_webSocket, &QWebSocket::textMessageReceived, [](const QString& message)
	{
		qDebug() << "textMessageReceived " << message;
	});
	QObject::connect(m_webSocket, &QWebSocket::binaryMessageReceived, [](const QByteArray &message)
	{
		qDebug() << "binaryMessageReceived " << message;
	});
	QObject::connect(m_webSocket, &QWebSocket::error, [](QAbstractSocket::SocketError error)
	{
		qDebug() << "error " << error;
	});
	QObject::connect(m_webSocket, &QWebSocket::pong, [](quint64 elapsedTime, const QByteArray& payload)
	{
		qDebug() << "pong " << elapsedTime << payload;
	});
	QObject::connect(m_webSocket, &QWebSocket::bytesWritten, [](qint64 bytes)
	{
		qDebug() << "bytesWritten " << bytes;
	});*/
}
