#pragma once
#include <memory>

class Inbox {
public:
    Inbox();
    ~Inbox(); // must be defined in .cpp
private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
