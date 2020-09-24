#include "MyTextEdit.h"
#include <QPainter>
#include <QKeyEvent>
MyTextEdit::MyTextEdit(QWidget* parent) : QTextEdit(parent)
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
		painter.fillRect(rect, (it->second)->getCursorColor());
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
	emit clickOnTextEdit(event);
}

void MyTextEdit::keyPressEvent(QKeyEvent* e)
{
	switch (e->key()) {
	case Qt::Key_Up:
	case Qt::Key_Down:
	case Qt::Key_Left:
	case Qt::Key_Right: {
		QTextEdit::keyPressEvent(e);
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
			it->second->setCursorPosition(cursorPosition + offsetPosition);
		}
	}
}

void MyTextEdit::moveBackwardCursorsPosition(int mainCursorPosition, int offsetPosition) {
	for (auto it = m_cursorsToPrint.begin(); it != m_cursorsToPrint.end(); it++) {
		int cursorPosition = it->second->getCursorPosition();
		if (cursorPosition > mainCursorPosition && cursorPosition - offsetPosition >= 0) {
			it->second->setCursorPosition(cursorPosition - offsetPosition);
		}
		else if (cursorPosition > mainCursorPosition && cursorPosition - offsetPosition < 0) {
			it->second->setCursorPosition(0);
		}
	}
}
