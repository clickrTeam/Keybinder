#pragma once

#include "event.h"
#include <chrono>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>

struct KeyQueueState {
    std::deque<KeyEvent> queue;
    std::condition_variable cv;
    std::mutex mtx;
};

// TODO: have some kind of filter to easily send keys back when they are not
// being modified
class KeySender {
  public:
    explicit KeySender(const std::shared_ptr<KeyQueueState> &state);
    // Return true if the key was used. If false is returned then the key is not
    // passed to the mapper
    bool send_key(KeyEvent kc);

  private:
    std::shared_ptr<KeyQueueState> state;
};

class KeyReceiver {
  public:
    explicit KeyReceiver(const std::shared_ptr<KeyQueueState> &state);

    // Wait for a key with optional timeout
    // If timeout is not provided, waits indefinitely
    std::optional<KeyEvent>
    wait_key(std::optional<std::chrono::milliseconds> timeout = std::nullopt);

  private:
    std::shared_ptr<KeyQueueState> state;
};

// Factory to create a sender/receiver pair sharing the same queue
std::pair<KeySender, KeyReceiver> create_channel();
