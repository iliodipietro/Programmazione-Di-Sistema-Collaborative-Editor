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
    ClientManager(QTcpSocket* socket, QObject *parent = nullptr);
    void setUsername(QString username);
    void setColor(QColor color);
    void setId(int id);
    bool writeData(QByteArray& data);
    QString getUsername();
    int getId();
    QTcpSocket* getSocket();
    ~ClientManager();

signals:
    void messageReceived(ClientManager*, QByteArray);
    void disconnected();

public slots:
    void readyRead();
    void onDisconnect();

private:
    QColor m_color;
    QString m_username;
    int m_id;
    QSharedPointer<QTcpSocket> m_clientSocket;
    QSharedPointer<QByteArray> m_socketBuffer;

    qint64 arrayToInt(QByteArray source);
    QByteArray intToArray(qint64 source);
};

#endif // CLIENTMANAGER_H
