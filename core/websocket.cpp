#include "websocket.h"
#include "logger.h"

namespace Core
{

namespace
{

QString serialize(const QJsonObject& object)
{
	return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

QJsonObject deserialize(const QString& message)
{
	return QJsonDocument::fromJson(message.toUtf8()).object();
}

}

WebSocket::WebSocket(const QUrl& url, QObject* parent)
	: QObject(parent)
	, m_queryId(0)
{
	connect(&m_webSocket, &QWebSocket::connected, this, &WebSocket::onConnected);
	connect(&m_webSocket, &QWebSocket::disconnected, this, &WebSocket::onDisconnected);

	m_webSocket.open(url);
}

int WebSocket::sendMessage(const QJsonObject& originalMessage)
{
	const int queryId = generateQueryId();

	QJsonObject message = originalMessage;
	message["queryId"] = queryId;

	LOG << "Send message: " << serialize(message);
	m_webSocket.sendTextMessage(serialize(message));

	return queryId;
}

void WebSocket::onConnected()
{
	connect(&m_webSocket, &QWebSocket::textFrameReceived, this, &WebSocket::onTextFrameReceived);

	LOG << "connected";
	emit connected();
}

void WebSocket::onDisconnected()
{
	LOG << "disconnected";
	emit disconnected();
}

void WebSocket::onTextFrameReceived(const QString& frame, bool isLastFrame)
{
	LOG << "Received text frame: " << frame << "; isLastFrame: " << isLastFrame;
	emit messageReceived(deserialize(frame));
}

int WebSocket::generateQueryId()
{
	return m_queryId++;
}

}
