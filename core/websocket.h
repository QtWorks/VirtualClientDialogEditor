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

	int sendMessage(const QJsonObject& message);

signals:
	void connected();
	void disconnected();
	void messageReceived(const QJsonObject& message);

private slots:
	void onConnected();
	void onDisconnected();
	void onTextFrameReceived(const QString& frame, bool isLastFrame);

private:
	int generateQueryId();

private:
	QWebSocket m_webSocket;
	int m_queryId;
};

}

#endif // WEBSOCKET_H
