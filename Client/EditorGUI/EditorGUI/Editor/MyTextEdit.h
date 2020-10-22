#pragma once
#include <QObject>
#include <QTextEdit>
#include <QColor>
#include <map>
#include "CustomCursor.h"

struct Interval{
    QRect rect;
    QColor color;

    Interval(QRect rect, QColor color) :rect(rect), color(color) {};
};

struct IntervalToPrint {
    QRect rect;
    QColor color;

    IntervalToPrint(QRect rect, QColor color) : rect(rect), color(color) {};
};

class Editor;

class MyTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    MyTextEdit(CRDT* crdt, QWidget* parent = 0);

    void addCursor(int id, QColor color, QString username, int position);
    void removeCursor(int id);
    void handleMessage(int id, Message& m, int position);
    void updateTextSize();
    void moveForwardCursorsPosition(int mainCursorPosition, int offsetPosition);
    void moveBackwardCursorsPosition(int mainCursorPosition, int offsetPosition);
    inline int getCursorPos(int id) { return m_cursorsToPrint.at(id)->getCursorPosition(); }
    void refresh(QKeyEvent* e);
    inline __int64 getEffectiveCursorPosition(std::vector<int> cursorPositionInCRDT) { return m_crdt->getCursorPosition(cursorPositionInCRDT); }

private:

    std::map<int, CustomCursor*> m_cursorsToPrint;
    std::vector<Interval> m_rowDimensions;
    std::vector<IntervalToPrint> m_intervalsToPrint;
    bool m_mousePress;
    bool m_usersIntervalsEnabled;
    bool m_usersIntervalsRepaint;
    bool m_rectAlreadyDone;
    CRDT* m_crdt;
    Editor* m_parentEditor;

    void paintCustomCursors();
    void paintUsersIntervals();

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* e);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

signals:
    void textSizeChanged();
    void clickOnTextEdit(QMouseEvent*);
    void propaga(QKeyEvent* e);
    void updateCursorPosition(bool);

private slots:
    void showHideUsersIntervals();

public slots:
    void updateUsersIntervals();
};

