#ifndef MYSERVER_H
#define MYSERVER_H

#include <QObject>


class MyServer : public QObject{
    Q_OBJECT
public:
    MyServer(QObject *parent = nullptr);
    ~MyServer();

private:
};

#endif // MYSERVER_H
