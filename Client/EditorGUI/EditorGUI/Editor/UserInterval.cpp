#include "UserInterval.h"

UserInterval::UserInterval(int userId, int start): m_userId(userId), m_startPosition(start), m_endPosition(start + 1){}

UserInterval::UserInterval(int userId, int start, int end) : m_userId(userId), m_startPosition(start), m_endPosition(end) {}