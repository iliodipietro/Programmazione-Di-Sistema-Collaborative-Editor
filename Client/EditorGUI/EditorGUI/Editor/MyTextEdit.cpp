#include "MyTextEdit.h"
#include "Editor.h"
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>
#include <locale>
#include <codecvt>

MyTextEdit::MyTextEdit(CRDT* crdt, QWidget* parent) : m_crdt(crdt), QTextEdit(parent), m_mousePress(false), m_usersIntervalsEnabled(false), m_rectAlreadyDone(false),
m_lastAction(false)
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
		QPair<bool, QRect> hideRect = it->second->getCursorPos();
		if (!hideRect.first) {
			hideRect.second.setX(hideRect.second.x() - 1);
			painter.fillRect(hideRect.second, QBrush((it->second)->getCursorColor()));
		}
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
	CustomCursor* cursor = new CustomCursor(m_parentEditor, this, color, username, position, m_crdt, this);
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
	m_lastAction = true;
	QScrollBar* SB = this->verticalScrollBar();
	int sbPos;
	if (SB != Q_NULLPTR) {
		sbPos = SB->value();
	}
	CustomCursor* cursor = m_cursorsToPrint.find(id)->second;
	cursor->messageHandler(m, position);
	if (SB != Q_NULLPTR) {
		SB->setValue(sbPos);
	}
	if (m.getAction() == CURSOR_S) {
		this->repaint();
	}
}

void MyTextEdit::updateTextSize() {
	emit textSizeChanged();
}

void MyTextEdit::scrollContentsBy(int dx, int dy) {
	if (!m_lastAction)
		updateUsersIntervals();
		QTextEdit::scrollContentsBy(dx, dy);
	m_lastAction = false;
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
				int interval = it->getEndPosition() - it->getStartPosition();
				if (interval >= 20) {
					int numOfInterval = interval / 20;
					int offset;
					int start = it->getStartPosition();
					int end = it->getStartPosition() + 20;
					int i = 1;
					while (i - 1 < numOfInterval) {
						TC.setPosition(end);
						QRect intRect = this->cursorRect(TC);
						while (intRect.y() == startRect.y() && end + 20 < it->getEndPosition()) {
							end += 20;
							i++;
							TC.setPosition(end);
							intRect = this->cursorRect(TC);
						}
						start = preciseIntervals(startRect, start, i, m_cursorsToPrint.at(it->getUserId())->getCursorColor());
						TC.setPosition(start);
						startRect = this->cursorRect(TC);
						interval = it->getEndPosition() - start;
						numOfInterval = interval / 20;
						end = start + 20;
						i = 1;
					}
					remainingIntervals(startRect, start, it->getEndPosition(), m_cursorsToPrint.at(it->getUserId())->getCursorColor());
				}
				else {
					remainingIntervals(startRect, it->getStartPosition(), it->getEndPosition(), m_cursorsToPrint.at(it->getUserId())->getCursorColor());
				}

			}
		}
	}

	if (m_usersIntervalsEnabled)
		this->repaint();
}

int MyTextEdit::preciseIntervals(QRect start, int startPos, int nIntervals, QColor color) {
	QRect intRect;
	QTextCursor TC = this->textCursor();
	int i;
	for (i = startPos + (nIntervals - 1) * 20; i < startPos + nIntervals * 20; i++) {
		TC.setPosition(i);
		intRect = this->cursorRect(TC);
		if (intRect.y() > start.y())
		{
			TC.setPosition(i - 1);
			intRect = this->cursorRect(TC);
			start.setWidth(intRect.x() - start.x());
			m_rowDimensions.emplace_back(start, color);
			return i;
		}
	}
	TC.setPosition(i);
	intRect = this->cursorRect(TC);
	start.setWidth(intRect.x() - start.x());
	m_rowDimensions.emplace_back(start, color);
	return i;
}

void MyTextEdit::remainingIntervals(QRect start, int startPos, int endPos, QColor color) {
	if (startPos == endPos) return;
	QRect intRect;
	QTextCursor TC = this->textCursor();
	bool flag = false;
	for (int i = startPos; i < endPos + 1; i++) {
		TC.setPosition(i);
		intRect = this->cursorRect(TC);
		int y = intRect.y();
		if (intRect.y() != start.y()) {
			TC.setPosition(i - 1);
			intRect = this->cursorRect(TC);
			start.setWidth(intRect.x() - start.x());
			m_rowDimensions.emplace_back(start, color);
			TC.setPosition(i);
			start = this->cursorRect(TC);
			if(i == endPos)
				flag = true;
		}
	}
	if (!flag) {
		start.setWidth(intRect.x() - start.x());
		m_rowDimensions.emplace_back(start, color);
	}
}

/*TC.setPosition(it->getStartPosition(), QTextCursor::MoveAnchor);
				TC.setPosition(it->getEndPosition(), QTextCursor::KeepAnchor);
				QString str = TC.selectedText();
				bool multipleLines = str.contains(QRegularExpression(QStringLiteral("[\\x{2029}]")));
				do {
					TC.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 1);
					QRect newRect = this->cursorRect(TC);
				}
				while
				int pos = TC.position();
				TC.setPosition(it->getStartPosition());
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
				m_rowDimensions.emplace_back(endRect, m_cursorsToPrint.at(it->getUserId())->getCursorColor());*/