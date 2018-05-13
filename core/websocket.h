#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <QObject>
#include <QtWebSockets/QtWebSockets>

namespace Core
{

class WebSocket
	: public QObject
{
	Q_OBJECT

public:
	WebSocket(const QUrl& url, QObject* parent = 0);
	~WebSocket();

	int sendMessage(const QJsonObject& message);

signals:
	void connected();
	void disconnected();
	void messageReceived(const QJsonObject& message);
	void error(const QString& errorMessage);

private slots:
	void onConnected();
	void onDisconnected();
	void onError(QAbstractSocket::SocketError error);
	void onTextFrameReceived(const QString& frame, bool isLastFrame);

private:
	int generateQueryId();

private:
	QUrl m_url;
	QWebSocket m_webSocket;
	int m_queryId;

	QVector<QJsonObject> m_pendingMessages;
};

}

#endif // WEBSOCKET_H
