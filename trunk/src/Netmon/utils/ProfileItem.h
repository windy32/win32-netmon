#ifndef PROFILE_ITEM_H
#define PROFILE_ITEM_H

// Base class
class ProfileValueItem
{
public:
    virtual ~ProfileValueItem() {};
    virtual ProfileValueItem *Parse(const std::tstring &text) = 0;;
    virtual std::tstring ToString() = 0;
};

// Derived classes
class ProfileBoolItem : public ProfileValueItem
{
public:
    bool value;

public:
    ProfileBoolItem(bool value);
    virtual ProfileValueItem *Parse(const std::tstring &text);
    virtual std::tstring ToString();
};

class ProfileIntItem : public ProfileValueItem
{
public:
    int value;

public:
    ProfileIntItem(int value);
    virtual ProfileValueItem *Parse(const std::tstring &text);
    virtual std::tstring ToString();
};

class ProfileStringItem : public ProfileValueItem
{
public:
    std::tstring value;

public:
    ProfileStringItem(const std::tstring &value);
    virtual ProfileValueItem *Parse(const std::tstring &text);
    virtual std::tstring ToString();
};

class ProfileIntListItem : public ProfileValueItem
{
public:
    std::vector<int> value;

public:
    ProfileIntListItem();
    ProfileIntListItem(const std::vector<int> &value);
    virtual ProfileValueItem *Parse(const std::tstring &text);
    virtual std::tstring ToString();
};

#endif
