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
    void handleMessage(int id, Message& m, int position);
    void updateTextSize();
    void moveForwardCursorsPosition(int mainCursorPosition, int offsetPosition);
    void moveBackwardCursorsPosition(int mainCursorPosition, int offsetPosition);
    inline int getCursorPos(int id) { return m_cursorsToPrint.at(id)->getCursorPosition(); }
    void refresh(QKeyEvent* e);

private:

    std::map<int, CustomCursor*> m_cursorsToPrint;

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent *e);

signals:
    void textSizeChanged();
    void clickOnTextEdit(QMouseEvent* e);
    void propaga(QKeyEvent* e);

};

