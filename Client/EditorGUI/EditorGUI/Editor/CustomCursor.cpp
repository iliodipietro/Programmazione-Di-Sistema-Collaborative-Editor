#include "CustomCursor.h"
#include <QPainter>
#include <QWindow>
#include <QDebug>

CustomCursor::CustomCursor(QTextEdit* editor, QColor color, QString username, int position, CRDT* crdt, QObject* parent) : m_editor(editor),
m_color(color), m_position(position), QObject(parent), m_username(username), m_usernameLabel(new QLabel(username, editor)), m_TextCursor(new QTextCursor(editor->document())),
m_textDoc(editor->document()), m_endSelection(-1), m_startSelection(-1), m_crdt(crdt)
{
	QTextCursor TCPrevious = m_editor->textCursor();
	m_usernameLabel->setAutoFillBackground(true);
	m_usernameLabel->setAlignment(Qt::AlignCenter);
	QPalette palette = m_usernameLabel->palette();
	palette.setColor(QPalette::Window, m_color);
	palette.setColor(m_usernameLabel->foregroundRole(), Qt::white);
	//m_usernameLabel->setPalette(palette);
	m_TextCursor->setPosition(position);
	m_editor->setTextCursor(*m_TextCursor);
	m_lastPosition = m_editor->cursorRect();
	QPoint cursorPos = m_lastPosition.topLeft();
	cursorPos.setY(cursorPos.y() - 2);
	cursorPos.setX(cursorPos.x() + 9);
	m_usernameLabel->move(cursorPos);
	m_usernameLabel->setAttribute(Qt::WA_StyledBackground);
	m_usernameLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
	m_usernameLabel->setStyleSheet("QLabel{border-radius: 3px; background:" + m_color.name() + "; color: white;}");
	m_usernameLabel->setContentsMargins(QMargins(4, 1, 4, 2));
	m_usernameLabel->show();
	m_editor->setTextCursor(TCPrevious);
}

void CustomCursor::messageHandler(Message& m, int index) {
	switch (m.getAction()) {
	case INSERT:
		updateViewAfterInsert(m, index);
		break;
	case DELETE_S:
		updateViewAfterDelete(m, index);
		break;
	case CHANGE:
		updateViewAfterStyleChange(m, index);
		break;
	case CURSOR_S:
		setCursorPosition(m_crdt->getCursorPosition(m.getCursorPosition()), ChangePosition, m.getIsSelection());
		break;
	default:
		break;
	}
	updateLabelPosition();
}

void CustomCursor::setCursorPosition(int pos, CursorMovementMode mode, bool isSelection) {
	switch (mode) {
	case AfterDelete:
		m_position = pos - 1;
		m_TextCursor->setPosition(pos);
		break;
	case AfterInsert:
		m_position = pos + 1;
		m_TextCursor->setPosition(pos);
		break;
	case ChangePosition:
		if (isSelection) {
			if (m_startSelection == -1) {
				m_startSelection = m_position;
				m_endSelection = pos;
			}
			else {
				m_endSelection = pos;
			}
		}
		else {
			m_startSelection = m_endSelection = -1;
		}
		m_position = pos;
		m_TextCursor->setPosition(pos);
		m_editor->setTextCursor(*m_TextCursor);
		updateLabelPosition();
		return;
	}
	m_editor->setTextCursor(*m_TextCursor);
}

QRect CustomCursor::getCursorPos() {
	return m_lastPosition;
}

void CustomCursor::setActiveCursor() {
	m_editor->setTextCursor(*m_TextCursor);
}

void CustomCursor::updateLabelPosition() {
	m_lastPosition = m_editor->cursorRect();
	QPoint cursorPos = m_lastPosition.topLeft();
	cursorPos.setY(cursorPos.y() - 2);
	cursorPos.setX(cursorPos.x() + 9);
	m_usernameLabel->move(cursorPos);
	m_usernameLabel->show();
}

void CustomCursor::updateViewAfterInsert(Message m, __int64 index)
{

	//retrieving remote state
	QChar chr(m.getSymbol().getChar());
	QFont r_font = m.getSymbol().getFont();
	QColor r_color = m.getSymbol().getColor();
	Qt::AlignmentFlag alignment = m.getSymbol().getAlignment();
	/// 
	/// la parte qui sotto potrbbe essere inutile per poter scrivere sul text editor 
	/// conviene usare la Qchartextedit--> vedi nota vocale su telegram a me stesso
	//o vedere changeViewAfterInsert in mainwindow.cpp debora

	//QTextCursor TC = m_editor->textCursor();
	// saving current state
	//QFont font = m_textEdit->currentFont();
	//QColor color = m_textEdit->textColor();

	//colore del char che arriva
	QTextCharFormat format;
	format.setFont(r_font);
	format.setForeground(r_color);
	//m_textEdit->setFont(r_font);
	//m_textEdit->setTextColor(r_color);

	setCursorPosition(index, AfterInsert);
	m_TextCursor->insertText(chr, format);

	QTextBlockFormat blockFormat = m_TextCursor->blockFormat();
	blockFormat.setAlignment(alignment);

	m_TextCursor->mergeBlockFormat(blockFormat);


	//ripristino
	//TC.setPosition(this->lastCursor);
	//m_textEdit->setFont(font);
	//m_textEdit->setTextColor(color);

}

void CustomCursor::updateViewAfterDelete(Message m, __int64 index)
{
	if (index == -1)
		return;//non devo fare niente in questo caso ho provato ad eliminare ma non ho trovato il carattere-->MADARE ERROR, ECCEZIONE??????

	//QTextCursor TC = m_editor->textCursor();
	setCursorPosition(index, AfterDelete);
	m_TextCursor->deleteChar();
	//TC.deletePreviousChar();//oppure è questo se il primo non funziona
}

void CustomCursor::updateViewAfterStyleChange(Message m, __int64 index)
{
	//QTextCursor TC = m_editor->textCursor();
	setCursorPosition(index, ChangePosition);
	m_TextCursor->deleteChar();

	QChar chr(m.getSymbol().getChar());
	QFont r_font = m.getSymbol().getFont();
	QColor r_color = m.getSymbol().getColor();
	Qt::AlignmentFlag alignment = m.getSymbol().getAlignment();

	//change color, font and style in general
	QTextCharFormat format;
	format.setFont(r_font);
	format.setForeground(r_color);
	m_TextCursor->mergeCharFormat(format);

	//change alignment
	QTextBlockFormat blockFormat = m_TextCursor->blockFormat();
	blockFormat.setAlignment(alignment);

	m_TextCursor->mergeBlockFormat(blockFormat);
	m_TextCursor->insertText(chr, format);
}

void CustomCursor::textSizeChanged() {
	m_TextCursor->setPosition(m_position);
	m_editor->setTextCursor(*m_TextCursor);
	updateLabelPosition();
}

int CustomCursor::getCursorPosition() {
	return m_position;
}

CustomCursor::~CustomCursor() {
	m_usernameLabel->deleteLater();
}