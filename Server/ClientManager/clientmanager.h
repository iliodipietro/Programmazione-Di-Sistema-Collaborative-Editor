#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include <QObject>
#include <QColor>
#include <QTcpSocket>
#include <QSharedPointer>

class ClientManager : public QObject
{
    Q_OBJECT
public:
    ClientManager(int id, QTcpSocket* socket, QObject *parent = nullptr);
    void setUsername(QString username);
    void setColor(QColor color);
    ~ClientManager();

signals:
    void messageReceived(QTcpSocket*, QByteArray);
    void disconnected();

public slots:
    void onNewConnection();
    void readyRead();
    void onDisconnect();

private:
    QColor m_color;
    QString m_username;
    int m_id;
    QSharedPointer<QTcpSocket> m_clientSocket;
    QSharedPointer<QByteArray> m_socketBuffer;
};

#endif // CLIENTMANAGER_H
