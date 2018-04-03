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
	explicit WebSocket(QObject* parent = 0);

signals:

public slots:

private:
	QWebSocket* m_webSocket;
};

}

#endif // WEBSOCKET_H
