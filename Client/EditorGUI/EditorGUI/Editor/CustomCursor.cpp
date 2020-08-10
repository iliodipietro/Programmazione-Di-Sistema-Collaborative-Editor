#include "CustomCursor.h"
#include <QPainter>
#include <QWindow>

CustomCursor::CustomCursor(QTextEdit* editor, QColor color, QString username, int position, QObject* parent): m_editor(editor),
	m_color(color), m_position(position), QObject(parent), m_username(username), m_usernameLabel(new QLabel(username, editor)), m_TextCursor(new QTextCursor(editor->document())),
	m_textDoc(editor->document())
{
	m_usernameLabel->setAutoFillBackground(true);
	m_usernameLabel->setAlignment(Qt::AlignCenter);
	QPalette palette = m_usernameLabel->palette();
	palette.setColor(QPalette::Window, m_color);
	palette.setColor(m_usernameLabel->foregroundRole(), Qt::white);
	//m_usernameLabel->setPalette(palette);
	m_usernameLabel->hide();
	m_TextCursor->setPosition(position);
}

void CustomCursor::messageHandler(Message& m, int index) {
	m_editor->setTextCursor(*m_TextCursor);
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
	default:
		break;
	}
}

void CustomCursor::setCursorPosition(int pos) {
	m_TextCursor->setPosition(pos);
	m_editor->setTextCursor(*m_TextCursor);
	updateLabelPosition();
}

QRect CustomCursor::getCursorPos() {
	return m_lastPosition;
}

void CustomCursor::setActiveCursor() {
	m_editor->setTextCursor(*m_TextCursor);
}

void CustomCursor::insertText(QString& text) {
	//m_textDoc = m_editor->document();
	//m_TextCursor = new QTextCursor(m_textDoc);
	//m_textDoc->setPlainText(m_editor->toPlainText());
	m_TextCursor->setPosition(10);
	m_editor->setTextCursor(*m_TextCursor);
	m_TextCursor->beginEditBlock();
	m_TextCursor->insertText(text);
	m_TextCursor->endEditBlock();
	m_lastPosition = m_editor->cursorRect();
	//m_editor->setPlainText(m_textDoc->toPlainText());
	updateLabelPosition();
}

void CustomCursor::updateLabelPosition() {
	QPoint cursorPos = m_lastPosition.topLeft();
	cursorPos.setY(cursorPos.y() - 2);
	cursorPos.setX(cursorPos.x() + 9);
	m_usernameLabel->move(cursorPos);
	m_usernameLabel->setAttribute(Qt::WA_StyledBackground);
	m_usernameLabel->setStyleSheet("QLabel{border-radius: 3px; background: blue; color: white;}");
	m_usernameLabel->setContentsMargins(QMargins(4, 1, 4, 2));
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

	m_TextCursor->setPosition(index);
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
	m_TextCursor->setPosition(index);
	m_TextCursor->deleteChar();
	//TC.deletePreviousChar();//oppure è questo se il primo non funziona
}

void CustomCursor::updateViewAfterStyleChange(Message m, __int64 index)
{
	//QTextCursor TC = m_editor->textCursor();
	m_TextCursor->setPosition(index);
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
	m_editor->setTextCursor(*m_TextCursor);
	m_lastPosition = m_editor->cursorRect();
	updateLabelPosition();
}