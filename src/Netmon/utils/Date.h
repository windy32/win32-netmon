#ifndef DATE_H
#define DATE_H

class ShortDate
{
public:
    int year;   // years since 1900
    int month;  // months since January - [0,11]

public:
    static ShortDate Null;

public:
    ShortDate();
    ShortDate(int year, int month);

public:
    ShortDate PrevMonth();
    ShortDate NextMonth();

public:
    bool operator<(const ShortDate &right) const;
    bool operator>(const ShortDate &right) const;
    bool operator==(const ShortDate &right) const;
};

class Date // hour, minute and second are 00:00:00
{
public:
    int year;   // years since 1900
    int month;  // months since January - [0,11]
    int mday;   // day of the month - [1,31]
    int wday;   // days since Sunday - [0,6]

public:
    Date(time_t time);
    Date(int year, int month, int day);

public:
    int ToInt32();
    ShortDate ToShortDate();
    static int GetTotalDays(int year, int month);
};

#endif
