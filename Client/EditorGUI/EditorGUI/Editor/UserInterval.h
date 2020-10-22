#pragma once

class UserInterval {

public:
	UserInterval(int userId, int start);
	UserInterval(int userId, int start, int end);
	inline int getUserId() { return m_userId; };
	inline int getIntervalLenght() { return m_endPosition - m_startPosition; };
	inline int getStartPosition() { return m_startPosition; };
	inline int getEndPosition() { return m_endPosition; };

	~UserInterval() {};

private:
	int m_userId;
	int m_startPosition;
	int m_endPosition;
};