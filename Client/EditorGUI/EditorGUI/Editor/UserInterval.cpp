#include "UserInterval.h"

UserInterval::UserInterval(int userId, int start): m_userId(userId), m_startPosition(start), m_endPosition(start + 1){}

UserInterval::UserInterval(int userId, int start, int end) : m_userId(userId), m_startPosition(start), m_endPosition(end) {}

void UserInterval::updateIntervalAfterInsert() {
	m_endPosition++;
}

void UserInterval::updateIntervalAfterDelete(int position) {
	if (m_endPosition >= position) m_endPosition--;
	else if (m_startPosition > position) {
		m_startPosition--;
		m_endPosition--;
	}
}

bool UserInterval::positionInInterval(int position) {
	if (position >= m_startPosition && position <= m_endPosition)
		return true;
	return false;
}

UserInterval UserInterval::splitInterval(int position) {
	UserInterval intervalAfter(m_userId, position + 1, m_endPosition + 1);
	m_endPosition = position;
	return intervalAfter;
}

void UserInterval::mergeIntervals(UserInterval& interval) {
	m_endPosition = interval.m_endPosition;
}