#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include <QObject>
#include <QColor>
#include <QTcpSocket>
#include <QSharedPointer>
#include <thread>
#include <vector>

class ClientManager : public QObject
{
    Q_OBJECT
public:
    ClientManager(QTcpSocket* socket, QObject *parent = nullptr);
    void setUsername(QString username);
    void setEmail(QString email);
    void setColor(QColor color);
    void setId(int id);
    bool writeData(QByteArray& data);


    QString getUsername();
    QString getEmail();
    int getId();
    inline  QColor getColor() { return m_color; }

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
    QString m_email;
    int m_id;
    QSharedPointer<QTcpSocket> m_clientSocket;
    QSharedPointer<QByteArray> m_socketBuffer;
    qint64 m_previousSize;
    bool m_readThreadRun;
    std::thread* m_readThread;
    std::vector<QByteArray> m_packetsInQueue;

    qint64 arrayToInt(QByteArray source);
    QByteArray intToArray(qint64 source);
    void readThreadFunction();
    void parseEmitMessages(QByteArray* message);
};

#endif // CLIENTMANAGER_H
