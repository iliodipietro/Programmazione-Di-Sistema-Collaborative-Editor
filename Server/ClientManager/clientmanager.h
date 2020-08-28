#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include <QObject>

class ClientManager : public QObject
{
    Q_OBJECT
public:
    explicit ClientManager(QObject *parent = nullptr);

signals:

public slots:
};

#endif // CLIENTMANAGER_H
