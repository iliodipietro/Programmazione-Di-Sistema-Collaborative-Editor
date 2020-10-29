#include "CustomCursor.h"
#include <QPainter>
#include <QWindow>
#include <QDebug>
#include "MyTextEdit.h"
#include "Editor.h"

#define TIME_TO_SHOW 150 //number of millisecond before repaint

CustomCursor::CustomCursor(Editor* widgetEditor, QTextEdit* editor, QColor color, int id, QString username, int position, CRDT* crdt, QObject* parent) : m_editor(editor),
m_color(color), m_position(position), QObject(parent), m_username(username), m_usernameLabel(new QLabel(username, editor)), m_TextCursor(new QTextCursor(editor->document())),
m_textDoc(editor->document()), m_crdt(crdt), m_widgetEditor(widgetEditor), m_hide(false), m_id(id)
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

	//------------------------------
	timer = new QTimer();
	timer->setSingleShot(true);
	timer->setInterval(TIME_TO_SHOW);
	//connect(timer, SIGNAL(timeout()), this, SLOT(paintNow()));
	m_parentEditor = qobject_cast<MyTextEdit*>(this->parent());

}

void CustomCursor::messageHandler(Message& m, int index) {
	switch (m.getAction()) {
	case INSERT:
		updateViewAfterInsert(m, index, "");
		emit updatePositionsAfterInsert(m_id, index);
		break;
	case DELETE_S:
		updateViewAfterDelete(m, index);
		emit updatePositionsAfterDelete(m_id, index);
		break;
	case CHANGE:
		updateViewAfterStyleChange(m, index, "");
		break;
	case CURSOR_S:
		setCursorPosition(m_crdt->getCursorPosition(m.getCursorPosition()), ChangePosition, m.getIsSelection());
		break;
	default:
		break;
	}
	//updateLabelPosition();
}

void CustomCursor::setCursorPosition(int pos, CursorMovementMode mode, int lenght) {
	switch (mode) {
	case AfterDelete:
		m_position = pos;
		m_TextCursor->setPosition(pos);
		break;
	case AfterInsert:
		m_position = pos + 1;
		m_TextCursor->setPosition(pos);
		break;
	case ChangePosition:
		m_position = pos;
		m_TextCursor->setPosition(pos);
		m_editor->setTextCursor(*m_TextCursor);
		//updateLabelPosition();
		return;
	}
	m_editor->setTextCursor(*m_TextCursor);
}

QPair<bool, QRect> CustomCursor::getCursorPos() {
	updateLabelPosition();
	return QPair<bool, QRect>(m_hide, m_lastPosition);
}

void CustomCursor::setActiveCursor() {
	m_editor->setTextCursor(*m_TextCursor);
}

void CustomCursor::updateLabelPosition() {
	m_TextCursor->setPosition(m_position);
	m_lastPosition = m_editor->cursorRect(*m_TextCursor);
	QPoint cursorPos = m_lastPosition.topLeft();
	QPoint cursorPosGlobal = m_editor->mapToParent(cursorPos);
	QRect m_editorSize = m_editor->geometry();
	if (m_editor->geometry().contains(m_editor->mapToParent(cursorPos))) {
		cursorPos.setY(cursorPos.y() - 2);
		cursorPos.setX(cursorPos.x() + 9);
		m_usernameLabel->move(cursorPos);
		m_usernameLabel->show();
		m_hide = false;
	}
	else {
		m_usernameLabel->hide();
		m_hide = true;
	}
}



void CustomCursor::updateViewAfterInsert(Message m, __int64 index, QString str)
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

void CustomCursor::updateViewAfterStyleChange(Message m, __int64 index, QString str)
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




QString CustomCursor::groupTogether()
{
	// entro in questa funzione solo se è una insert o una change style
	QString s;

	Message m = this->message_list.front();
	this->message_list.pop();
	this->index_list.pop();
	s.append(QChar(m.getSymbol().getChar()));

	while (!this->message_list.empty())
	{
		Message m2 = this->message_list.front();
		if (m2.getAction() == m.getAction() &&
			(m2.getSymbol().getColor() == m.getSymbol().getColor()) &&
			(m2.getSymbol().getFont() == m.getSymbol().getFont()) &&
			(m2.getSymbol().getAlignment() == m.getSymbol().getAlignment())) {

			s.append(QChar(m2.getSymbol().getChar()));
			this->message_list.pop();
			this->index_list.pop();
		}
		else {
			break;
		}

	}
	if (m.getAction() == CHANGE) {
		std::reverse(s.begin(), s.end());
	}

	return s;
}

void CustomCursor::paintNow()
{

	if (this->index_list.size() <= 0)
		return;

	QTextCursor TC = m_editor->textCursor();
	int pos = TC.position();
	QString str;
	//MyTextEdit* p = qobject_cast<MyTextEdit*>(this->parent());
	while (!this->index_list.empty()) {

		Message m = this->message_list.front();
		int index = this->index_list.front();

		//disegno ogni tot di caratteri e non sempre
		switch (m.getAction()) {
		case INSERT:
			str = groupTogether();
			updateViewAfterInsert(m, index, str);

			m_parentEditor->updateUsersIntervals();

			pos > index ? pos += str.length() : pos = pos;

			break;
		case DELETE_S:
			updateViewAfterDelete(m, index);
			this->message_list.pop();
			this->index_list.pop();
			m_parentEditor->updateUsersIntervals();

			pos > index ? pos-- : pos = pos;

			break;
		case CHANGE:
			str = groupTogether();
			updateViewAfterStyleChange(m, index, str);
			m_parentEditor->updateUsersIntervals();
			break;
			//case CURSOR_S:
			//	setCursorPosition(m_crdt->getCursorPosition(m.getCursorPosition()), ChangePosition, m.getIsSelection());
			//	this->message_list.pop();
			//	this->index_list.pop();
			//	break;
		default:
			break;
		}
	}
	updateLabelPosition();

	TC.setPosition(pos, QTextCursor::MoveAnchor);
	m_editor->setTextCursor(TC);
	m_parentEditor->repaint();
	//this->message_list.clear();
	//this->index_list.clear();
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