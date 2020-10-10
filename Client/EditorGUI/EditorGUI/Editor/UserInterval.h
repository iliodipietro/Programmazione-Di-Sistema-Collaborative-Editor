#pragma once

class UserInterval {

public:
	UserInterval(int userId, int start);
	UserInterval(int userId, int start, int end);
	void updateIntervalAfterInsert();
	void updateIntervalAfterDelete(int position);
	UserInterval splitInterval(int position);
	void mergeIntervals(UserInterval &interval);
	bool positionInInterval(int position);
	inline int getUserId() { return m_userId; };
	inline int getIntervalLenght() { return m_endPosition - m_startPosition; };

	~UserInterval() {};

private:
	int m_userId;
	int m_startPosition;
	int m_endPosition;
};