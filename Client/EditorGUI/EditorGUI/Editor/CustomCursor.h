#pragma once
#include <QObject>
#include <QTextEdit>
#include <QLabel>
#include "CRDT/CRDT.h"

class CustomCursor : public QObject
{
	Q_OBJECT
public:
	CustomCursor(QTextEdit* editor, QColor color, QString username, int position, QObject *parent = Q_NULLPTR);

	void messageHandler(Message &message, int position);
	void setCursorPosition(int pos);
	inline QColor getCursorColor() { return m_color; }
	QRect getCursorPos();
	void setActiveCursor();
	//void insertText(QString &text);
	void updateLabelPosition();

private:
	QTextEdit* m_editor;
	QTextDocument* m_textDoc;
	QTextCursor* m_TextCursor;
	QLabel* m_usernameLabel;
	QString m_username;
	QColor m_color;
	QRect m_lastPosition;
	int m_position;

	void updateViewAfterInsert(Message m, __int64 index);
	void updateViewAfterDelete(Message m, __int64 index);
	void updateViewAfterStyleChange(Message m, __int64 index);

public slots:
	void textSizeChanged();
};

