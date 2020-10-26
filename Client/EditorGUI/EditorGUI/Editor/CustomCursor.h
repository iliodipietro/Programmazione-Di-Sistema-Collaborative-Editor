#pragma once
#include <QObject>
#include <QTextEdit>
#include <QLabel>
#include "CRDT/CRDT.h"
#include "QTimer"
#include <queue>
#include <QPair>

class Editor;
class MyTextEdit;

class CustomCursor : public QObject
{
	Q_OBJECT
public:

	enum CursorMovementMode { AfterDelete, AfterInsert, ChangePosition };

	CustomCursor(Editor* widgetEditor, QTextEdit* editor, QColor color, QString username, int position, CRDT* crdt, QObject* parent = Q_NULLPTR);
	~CustomCursor();

	void messageHandler(Message& message, int position);
	void setCursorPosition(int pos, CursorMovementMode mode, int lenght = 0);
	inline QColor getCursorColor() { return m_color; }
	QPair<bool, QRect> getCursorPos();
	int getCursorPosition();
	void setActiveCursor();
	void updateLabelPosition();
	
private:

	Editor* m_widgetEditor;
	MyTextEdit* m_parentEditor;
	QTextEdit* m_editor;
	QTextDocument* m_textDoc;
	QTextCursor* m_TextCursor;
	QLabel* m_usernameLabel;
	QString m_username;
	QColor m_color;
	QRect m_lastPosition;
	CRDT* m_crdt;
	int m_position;
	bool m_hide;

	void updateViewAfterInsert(Message m, __int64 index, QString str);
	void updateViewAfterDelete(Message m, __int64 index);
	void updateViewAfterStyleChange(Message m, __int64 index, QString str);

	QTimer* timer;
	std::queue<Message> message_list;
	std::queue<int> index_list;
	QString groupTogether();


public slots:
	void textSizeChanged();
	void paintNow();
};

