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
	, m_url(url)
	, m_queryId(0)
{
	connect(&m_webSocket, &QWebSocket::connected, this, &WebSocket::onConnected);
	connect(&m_webSocket, &QWebSocket::disconnected, this, &WebSocket::onDisconnected);
	connect(&m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &WebSocket::onError);

	connect(&m_webSocket, &QWebSocket::stateChanged, [](QAbstractSocket::SocketState state) { LOG << "Websocket state changed to " << state; });
}

WebSocket::~WebSocket()
{
	m_webSocket.abort();
}

int WebSocket::sendMessage(const QJsonObject& originalMessage)
{
	QJsonObject message = originalMessage;

	if (!originalMessage.contains("queryId"))
	{
		const int queryId = generateQueryId();
		message["queryId"] = queryId;
	}

	const int queryId = message["queryId"].toInt();

	if (m_webSocket.state() != QAbstractSocket::ConnectedState)
	{
		LOG << "Socket is closed, push to pending";

		m_pendingMessages.push_back(message);

		if (m_webSocket.state() != QAbstractSocket::ConnectingState)
		{
			LOG << "Socket is closed, open";
			m_webSocket.open(m_url);
		}
		return queryId;
	}

	LOG << "Send message: " << serialize(message);
	m_webSocket.sendTextMessage(serialize(message));

	return queryId;
}

void WebSocket::onConnected()
{
	LOG << "Socket opened, pop all pending queries";

	connect(&m_webSocket, &QWebSocket::textFrameReceived, this, &WebSocket::onTextFrameReceived);

	while (!m_pendingMessages.isEmpty())
	{
		auto message = m_pendingMessages.takeFirst();
		sendMessage(message);
	}

	emit connected();
}

void WebSocket::onDisconnected()
{
	LOG << "Socket closed, interrupt all active queries";
	emit disconnected();
}

void WebSocket::onError(QAbstractSocket::SocketError errorCode)
{
	LOG << "Socket error" << ARG(errorCode) << ARG2(m_webSocket.errorString(), "errorString");
	emit error(m_webSocket.errorString());
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
