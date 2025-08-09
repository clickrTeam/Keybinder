#include "key_channel.h"
#include "event.h"

KeySender::KeySender(const std::shared_ptr<KeyQueueState> &state)
    : state(state) {}

bool KeySender::send_key(InputEvent kc) {
    {
        std::lock_guard<std::mutex> lock(state->mtx);
        state->queue.push_back(kc);
    }
    state->cv.notify_one();

    // TODO: Add a filter we check to see if the key needs to be sent. I plan to
    // add then when we move to a state machine
    return true;
}

KeyReceiver::KeyReceiver(const std::shared_ptr<KeyQueueState> &state)
    : state(state) {}

std::optional<InputEvent>
KeyReceiver::wait_key(std::optional<std::chrono::milliseconds> timeout) {
    std::unique_lock<std::mutex> lock(state->mtx);

    bool got_key;
    if (timeout.has_value()) {
        got_key = state->cv.wait_for(lock, *timeout,
                                     [this] { return !state->queue.empty(); });
    } else {
        state->cv.wait(lock, [this] { return !state->queue.empty(); });
        got_key = true;
    }

    if (!got_key) {
        return std::nullopt; // timed out
    }

    InputEvent kc = state->queue.front();
    state->queue.pop_front();
    return kc;
}

std::pair<KeySender, KeyReceiver> create_channel() {
    auto state = std::make_shared<KeyQueueState>();
    return {KeySender(state), KeyReceiver(state)};
}
