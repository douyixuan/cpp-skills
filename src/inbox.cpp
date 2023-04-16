#include <iostream>

#include "inbox.h"

struct Inbox::Impl {
    void run() {
        std::cout << "unbox and nothing\n";
    }
};

Inbox::Inbox() : impl(std::make_unique<Impl>()) {
    impl->run();
}

Inbox::~Inbox() {}
