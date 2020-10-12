#include "MyTextEdit.h"
#include "Editor.h"
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>
#include <locale>
#include <codecvt>

MyTextEdit::MyTextEdit(CRDT* crdt, QWidget* parent) : m_crdt(crdt), QTextEdit(parent), m_mousePress(false), m_usersIntervalsEnabled(false), m_rectAlreadyDone(false)
{
	Editor* editor = qobject_cast<Editor*>(this->parent());
	m_parentEditor = editor;
	connect(editor, &Editor::showHideUsersIntervalsSignal, this, &MyTextEdit::showHideUsersIntervals);
	connect(editor, &Editor::updateUsersIntervals, this, &MyTextEdit::updateUsersIntervals);
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
	QPainter painter(viewport());
	painter.setRenderHint(QPainter::Antialiasing, true);

	for (auto it = m_rowDimensions.begin(); it != m_rowDimensions.end(); it++) {
		QColor color = it->color;
		color.setAlpha(70);
		painter.fillRect(it->rect, QBrush(color));
	}
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
	//this->repaint();
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
		emit propaga(e);
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
	this->repaint();
}

void MyTextEdit::updateUsersIntervals() {
	auto usersCharactersIntervals = m_crdt->getUsersInterval();
	m_rowDimensions.clear();
	QTextCursor TC = this->textCursor();
	int initialPos = TC.position();

	for (auto it = usersCharactersIntervals->begin(); it != usersCharactersIntervals->end(); it++) {
		if (m_cursorsToPrint.find(it->getUserId()) != m_cursorsToPrint.end()) {
			TC.setPosition(it->getStartPosition());
			QRect startRect = this->cursorRect(TC);
			TC.setPosition(it->getEndPosition());
			QRect endRect = this->cursorRect(TC);
			if (startRect.y() == endRect.y()) {
				startRect.setWidth(endRect.x() - startRect.x());
				int pixelsHigh = startRect.height();
				m_rowDimensions.emplace_back(startRect, m_cursorsToPrint.at(it->getUserId())->getCursorColor());
			}
			else {
				TC.setPosition(it->getStartPosition(), QTextCursor::MoveAnchor);
				TC.setPosition(it->getEndPosition(), QTextCursor::KeepAnchor);
				QString str = TC.selectedText();
				bool multipleLines = str.contains(QRegularExpression(QStringLiteral("[\\x{2029}]")));
				if (multipleLines) {
					int initialPos = it->getStartPosition();
					do {
						int index = str.indexOf(QRegularExpression(QStringLiteral("[\\x{2029}]")));
						TC.setPosition(initialPos + index);
						QRect intRect = this->cursorRect(TC);
						startRect.setWidth(intRect.x() - startRect.x());
						m_rowDimensions.emplace_back(startRect, m_cursorsToPrint.at(it->getUserId())->getCursorColor());
						str = str.mid(index + 1);
						multipleLines = str.contains(QRegularExpression(QStringLiteral("[\\x{2029}]")));
						initialPos = initialPos + index + 1;
						TC.setPosition(initialPos);
						startRect = this->cursorRect(TC);
					} while (multipleLines);
				}
				else {
					startRect.setWidth(this->width());
					m_rowDimensions.emplace_back(startRect, m_cursorsToPrint.at(it->getUserId())->getCursorColor());
				}
				endRect.setX(4);
				m_rowDimensions.emplace_back(endRect, m_cursorsToPrint.at(it->getUserId())->getCursorColor());

			}
		}
	}

	if(m_usersIntervalsEnabled)
		this->repaint();
}