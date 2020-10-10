#include "MyTextEdit.h"
#include "Editor.h"
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>

MyTextEdit::MyTextEdit(CRDT* crdt, QWidget* parent) : m_crdt(crdt), QTextEdit(parent), m_mousePress(false), m_usersIntervalsEnabled(false)
{
	Editor* editor = qobject_cast<Editor*>(this->parent());
	connect(editor, &Editor::showHideUsersIntervalsSignal, this, &MyTextEdit::showHideUsersIntervals);
}

void MyTextEdit::paintEvent(QPaintEvent* event)
{
	QTextEdit::paintEvent(event);
	paintCustomCursors();
	if (m_usersIntervalsEnabled)
		paintUsersIntervals();
}

void MyTextEdit::paintCustomCursors() {
	QPainter painter(viewport());
	auto it = m_cursorsToPrint.begin();
	for (it; it != m_cursorsToPrint.end(); it++) {
		painter.setRenderHint(QPainter::Antialiasing, true);
		QRect rect = it->second->getCursorPos();
		rect.setX(rect.x() - 1);
		painter.fillRect(rect, QBrush((it->second)->getCursorColor()));
	}
}

void MyTextEdit::paintUsersIntervals() {
	Editor* editor = qobject_cast<Editor*>(parent());
	auto usersCharactersIntervals = editor->getUsersCharactersIntervals();
	std::vector<Interval> rowDimensions;
	QStringList strLst = this->toPlainText().split('\n');
	QFont textEditFont = this->font();
	QFontMetrics fm(textEditFont);

	int i = 0;
	for (auto it = usersCharactersIntervals->begin(); it != usersCharactersIntervals->end(); it++) {
		int intervalLength = it->getIntervalLenght();

		if (intervalLength < strLst[i].length()) {
			QString subStrs = strLst[i].left(intervalLength);
			int pixelsWide = fm.width(subStrs);
			int pixelsHigh = fm.height();
			rowDimensions.emplace_back(pixelsWide, pixelsHigh, i, m_cursorsToPrint.at(it->getUserId())->getCursorColor());
			strLst[i] = strLst[i].mid(intervalLength);
		}
		else if (intervalLength == strLst[i].length()) {
			int pixelsWide = fm.width(strLst[i]);
			int pixelsHigh = fm.height();
			rowDimensions.emplace_back(pixelsWide, pixelsHigh, i, m_cursorsToPrint.at(it->getUserId())->getCursorColor());
			i++;
		}
		else {
			int partialLength = intervalLength;
			do {
				partialLength -= strLst[i].length();
				int pixelsWide = fm.width(strLst[i]);
				int pixelsHigh = fm.height();
				rowDimensions.emplace_back(pixelsWide, pixelsHigh, i, m_cursorsToPrint.at(it->getUserId())->getCursorColor());
				i++;
			} while (partialLength > strLst[i].length());

			QString subStrs = strLst[i].left(partialLength);
			int pixelsWide = fm.width(subStrs);
			int pixelsHigh = fm.height();
			rowDimensions.emplace_back(pixelsWide, pixelsHigh, i, m_cursorsToPrint.at(it->getUserId())->getCursorColor());
			strLst[i] = strLst[i].mid(partialLength);
		}
	}

	QTextCursor TC = this->textCursor();
	int initialPos = TC.position();
	TC.movePosition(QTextCursor::Start);
	this->setTextCursor(TC);
	QPainter painter(viewport());

	for (auto it = rowDimensions.begin(); it != rowDimensions.end();) {
		QRect rect = this->cursorRect(TC);
		do {
			painter.setRenderHint(QPainter::Antialiasing, true);
			rect.setWidth(it->width);
			painter.fillRect(rect, QBrush((it->color)));
			rect.setX(it->width + 1);
			it++;
		} while (it != rowDimensions.end() && it->row == (it - 1)->row);
		TC.movePosition(QTextCursor::Down);
	}

	TC.setPosition(initialPos);
	this->setTextCursor(TC);
}

void MyTextEdit::addCursor(int id, QColor color, QString username, int position) {
	CustomCursor* cursor = new CustomCursor(this, color, username, position, m_crdt, this);
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
	if (m_mousePress)
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
		if (cursorPosition > mainCursorPosition && cursorPosition - offsetPosition >= mainCursorPosition) {
			it->second->setCursorPosition(cursorPosition - offsetPosition, CustomCursor::ChangePosition);
		}
		else if (cursorPosition > mainCursorPosition && cursorPosition - offsetPosition < mainCursorPosition) {
			it->second->setCursorPosition(mainCursorPosition, CustomCursor::ChangePosition);
		}
	}
}

void MyTextEdit::showHideUsersIntervals() {
	m_usersIntervalsEnabled = !m_usersIntervalsEnabled;
}