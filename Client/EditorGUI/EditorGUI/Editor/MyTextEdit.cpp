#include "MyTextEdit.h"
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>

MyTextEdit::MyTextEdit(QWidget* parent) : QTextEdit(parent), m_mousePress(false)
{
}

void MyTextEdit::paintEvent(QPaintEvent* event)
{
	QTextEdit::paintEvent(event);
	QPainter painter(viewport());
	auto it = m_cursorsToPrint.begin();
	for (it; it != m_cursorsToPrint.end(); it++) {
		painter.setRenderHint(QPainter::Antialiasing, true);
		QRect rect = it->second->getCursorPos();
		rect.setX(rect.x() - 1);
		painter.fillRect(rect, QBrush((it->second)->getCursorColor()));
	}
}

void MyTextEdit::addCursor(int id, QColor color, QString username, int position) {
	CustomCursor* cursor = new CustomCursor(this, color, username, position, this);
	m_cursorsToPrint.insert(std::pair<int, CustomCursor*>(id, cursor));
	connect(this, &MyTextEdit::textSizeChanged, cursor, &CustomCursor::textSizeChanged);
}

void MyTextEdit::removeCursor(int id) {
	CustomCursor* cursor = m_cursorsToPrint.find(id)->second;
	cursor->deleteLater();
	m_cursorsToPrint.erase(id);
	this->repaint();
}

void MyTextEdit::handleMessage(int id, Message& m, int position) {
	CustomCursor* cursor = m_cursorsToPrint.find(id)->second;
	cursor->messageHandler(m, position);
	this->repaint();
}

void MyTextEdit::updateTextSize() {
	emit textSizeChanged();
}

void MyTextEdit::refresh(QKeyEvent* e)
{
	QTextEdit::keyPressEvent(e);
}

void MyTextEdit::mousePressEvent(QMouseEvent* event) {
	QTextEdit::mousePressEvent(event);
	m_mousePress = true;
	emit clickOnTextEdit(event);
	emit updateCursorPosition(false);
}

void MyTextEdit::mouseReleaseEvent(QMouseEvent* event) {
	QTextEdit::mousePressEvent(event);
	m_mousePress = false;
}

void MyTextEdit::mouseMoveEvent(QMouseEvent* event) {
	QTextEdit::mousePressEvent(event);
	if(m_mousePress)
		emit updateCursorPosition(true);
}

void MyTextEdit::keyPressEvent(QKeyEvent* e)
{
	if (e->matches(QKeySequence::Copy)) {
		QTextEdit::keyPressEvent(e);
		return;
	}

	if (e->matches(QKeySequence::SelectNextChar) || e->matches(QKeySequence::SelectPreviousChar)) {
		QTextEdit::keyPressEvent(e);
		emit updateCursorPosition(true);
		return;
	}

	switch (e->key()) {
	case Qt::Key_Up:
	case Qt::Key_Down:
	case Qt::Key_Left:
	case Qt::Key_Right: {
		QTextEdit::keyPressEvent(e);
		emit updateCursorPosition(false);
		break;
	}
	default:
		emit propaga(e);
		break;
	}
}

void MyTextEdit::moveForwardCursorsPosition(int mainCursorPosition, int offsetPosition) {
	for (auto it = m_cursorsToPrint.begin(); it != m_cursorsToPrint.end(); it++) {
		int cursorPosition = it->second->getCursorPosition();
		if (cursorPosition > mainCursorPosition) {
			it->second->setCursorPosition(cursorPosition + offsetPosition, CustomCursor::ChangePosition);
		}
	}
}

void MyTextEdit::moveBackwardCursorsPosition(int mainCursorPosition, int offsetPosition) {
	for (auto it = m_cursorsToPrint.begin(); it != m_cursorsToPrint.end(); it++) {
		int cursorPosition = it->second->getCursorPosition();
		if (cursorPosition > mainCursorPosition && cursorPosition - offsetPosition >= 0) {
			it->second->setCursorPosition(cursorPosition - offsetPosition, CustomCursor::ChangePosition);
		}
		else if (cursorPosition > mainCursorPosition && cursorPosition - offsetPosition < 0) {
			it->second->setCursorPosition(0, CustomCursor::ChangePosition);
		}
	}
}
