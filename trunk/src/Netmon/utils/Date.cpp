#include "stdafx.h"
#include "Date.h"

// Short Date ////////////////////////////////////////////////////////////////
ShortDate ShortDate::Null;

ShortDate::ShortDate()
{
    this->year = -1;
    this->month = -1;
}

ShortDate::ShortDate(int year, int month)
{
    this->year = year;
    this->month = month;
}

ShortDate ShortDate::PrevMonth()
{
    ShortDate prev = *this;

    if (prev.month == 0)
    {
        prev.year -= 1;
        prev.month = 11;
    }
    else
    {
        prev.month -= 1;
    }
    return prev;
}

ShortDate ShortDate::NextMonth()
{
    ShortDate prev = *this;

    if (prev.month == 11)
    {
        prev.year += 1;
        prev.month = 0;
    }
    else
    {
        prev.month += 1;
    }
    return prev;
}

bool ShortDate::operator<(const ShortDate &right) const
{
    return (year < right.year) || (year == right.year && month < right.month);
}

bool ShortDate::operator>(const ShortDate &right) const
{
    return (year > right.year) || (year == right.year && month > right.month);
}

bool ShortDate::operator==(const ShortDate &right) const
{
    return (year == right.year) && (month == right.month);
}

// Date //////////////////////////////////////////////////////////////////////
Date::Date(time_t time)
{
    struct tm tmTime;
    localtime_s(&tmTime, &time);

    this->year = tmTime.tm_year;
    this->month = tmTime.tm_mon;
    this->mday = tmTime.tm_mday;
    this->wday = tmTime.tm_wday;
}

Date::Date(int year, int month, int day)
{
    // Build the tm structure
    struct tm tmTime;
    tmTime.tm_year = year;
    tmTime.tm_mon = month;
    tmTime.tm_mday = day;
    tmTime.tm_hour = 0;
    tmTime.tm_min = 0;
    tmTime.tm_sec = 0;
    tmTime.tm_isdst = 0;

    // Convert to time_t
    time_t t = mktime(&tmTime);

    // Convert to tm structure again
    localtime_s(&tmTime, &t);

    // Save
    this->year = tmTime.tm_year;
    this->month = tmTime.tm_mon;
    this->mday = tmTime.tm_mday;
    this->wday = tmTime.tm_wday;
}

int Date::ToInt32()
{
    struct tm tmTime;
    tmTime.tm_year = year;
    tmTime.tm_mon = month;
    tmTime.tm_mday = mday;

    tmTime.tm_hour = 0;
    tmTime.tm_min = 0;
    tmTime.tm_sec = 0;
    tmTime.tm_isdst = 0;
    
    return (int)mktime(&tmTime);
}

ShortDate Date::ToShortDate()
{
    return ShortDate(year, month);
}

int Date::GetTotalDays(int year, int month)
{
    int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Leap year
    if ((year % 4 == 0) && (year % 100 != 0 || year % 400 == 0))
    {
        days[1] = 29;
    }

    return days[month];
}
