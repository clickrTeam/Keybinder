#include "daemon.h"
#include <iostream>

Daemon::Daemon(Mapper &m) : mapper(m) {
    std::cout << "Daemon created" << std::endl;
}

Daemon::~Daemon() { std::cout << "Daemon destroyed" << std::endl; }

void Daemon::start() { std::cout << "Daemon started" << std::endl; }

void Daemon::cleanup() { std::cout << "Daemon cleaned up" << std::endl; }

void Daemon::send_key(int vk) { std::cout << "Key sent" << std::endl; }
