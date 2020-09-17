#pragma once
#include <QObject>
#include <QTextEdit>
#include <map>
#include "CustomCursor.h"

class MyTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    MyTextEdit(QWidget* parent = 0);

    void addCursor(int id, QColor color, QString username, int position);
    void removeCursor(int id);
    //void insertText(int id, QString& text);
    void handleMessage(int id, Message& m, int position);
    void updateTextSize();
    void setCursorPosition(int id, int position);

private:

    std::map<int, CustomCursor*> m_cursorsToPrint;

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent *e);
signals:
    void textSizeChanged();
    void clickOnTextEdit();
   // void propaga(int key);

};

