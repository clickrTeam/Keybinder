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

    bool ready;
    if (timeout.has_value()) {
        ready = state->cv.wait_for(lock, *timeout, [this] {
            return state->closed || !state->queue.empty();
        });
    } else {
        state->cv.wait(
            lock, [this] { return state->closed || !state->queue.empty(); });
        ready = true;
    }

    if (state->closed)
        return std::nullopt; // shutdown signal

    if (!ready || state->queue.empty())
        return std::nullopt;

    InputEvent e = state->queue.front();
    state->queue.pop_front();
    return e;
}

std::pair<KeySender, KeyReceiver> create_channel() {
    auto state = std::make_shared<KeyQueueState>();
    return {KeySender(state), KeyReceiver(state)};
}

void KeyReceiver::close() {
    {
        std::lock_guard<std::mutex> lock(state->mtx);
        state->closed = true;
    }
    state->cv.notify_all();
}
