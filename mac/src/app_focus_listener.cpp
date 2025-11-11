#include "app_focus_listener.h"
#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <regex>

AppFocusListener::AppFocusListener(Callback cb)
    : callback(std::move(cb)), stopFlag(false) {}

void AppFocusListener::start() {
    listenerThread = std::thread([this]() { run(); });
}

void AppFocusListener::stop() {
    stopFlag.store(true);
    if (listenerThread.joinable()) {
        listenerThread.join();
    }
}

AppFocusListener::~AppFocusListener() { stop(); }

void AppFocusListener::run() {
    std::unique_ptr<FILE, decltype(&pclose)> pipe(
        popen("lsappinfo listen +becameFrontmost forever", "r"), pclose);
    if (!pipe) {
        std::cerr << "Failed to start lsappinfo\n";
        return;
    }

    std::array<char, 512> buffer;
    std::string line;
    std::regex appRegex("affectedASN=\"([^\"]+)\"");

    while (!stopFlag.load() &&
           fgets(buffer.data(), buffer.size(), pipe.get())) {
        line = buffer.data();
        std::smatch match;
        if (std::regex_search(line, match, appRegex)) {
            QString appName = QString::fromStdString(match[1].str());
            callback(appName);
        }
    }
}
