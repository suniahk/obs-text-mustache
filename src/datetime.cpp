//
// Created by Mark Jones New on 2/3/2024.
//

#include <QString>
#include <sstream>
#include <string>
#include <ctime>
#include <iomanip>
#include "datetime.h"
using namespace std;

wstring getFormattedTime(const struct tm *const localTime, const wchar_t *format) {
	std::wstringstream wss;
	wss << std::put_time(localTime, format);

	return wss.str();
}

wstring getCurrentMonthName(const struct tm *const localTime)
{
	return getFormattedTime(localTime, L"%B");
}

wstring getCurrentYear(const struct tm *const localTime)
{
	return getFormattedTime(localTime, L"%EY");
}

wstring getCurrentDay(const struct tm *const localTime)
{
	return getFormattedTime(localTime, L"%e");
}

wstring getCurrentDayOfWeek(const struct tm *const localTime)
{
	return getFormattedTime(localTime, L"%A");
}

wstring getCurrent24Hour(const struct tm *const localTime)
{
	return getFormattedTime(localTime, L"%OH");
}

wstring getCurrent12Hour(const struct tm *const localTime)
{
	return getFormattedTime(localTime, L"%OI");
}

wstring getCurrentMinute(const struct tm *const localTime)
{
	return getFormattedTime(localTime, L"%M");
}

wstring getCurrentSecond(const struct tm *const localTime)
{
	return getFormattedTime(localTime, L"%S");
}

wstring getCurrentAmPm(const struct tm *const localTime)
{
	return getFormattedTime(localTime, L"%p");
}
