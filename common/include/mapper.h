#pragma once
#include "daemon.h"
#include "event.h"
#include "key_channel.h"
#include "key_counter.h"
#include "profile.h"
#include "settings.h"
#include "state_machine.h"
#include <QDebug>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <optional>
#include <vector>

using std::size_t;

class Mapper {
  public:
    Mapper(Profile, Daemon &, KeyReceiver, KeybinderSettings &, KeyCounter &,
           std::optional<std::function<void(QString)>> = std::nullopt);
    bool set_profile(Profile p);
    // This should be called in a seperate thread as it does not return
    void start();
    bool set_layer(size_t);
    bool waiting_for_timer();
    void stop();
    size_t processed_events();

  private:
    void queue_binds(const std::vector<Bind> &);
    void queue_output(OutputEvent, uint64_t);
    void set_layer_inner(size_t);
    void apply_transition(const Transition &,
                          std::optional<InputEvent> = std::nullopt);
    void check_queued_events();
    void process_input(InputEvent);
    std::mutex mtx;
    Profile profile;
    Daemon &daemon;
    KeyReceiver key_receiver;
    KeybinderSettings &settings;
    KeyCounter &key_counter;

    std::vector<std::vector<State>> states;
    size_t cur_layer_idx = 0;
    size_t cur_state_idx = HOME_STATE_IDX;
    size_t processed_events_count = 0;

    std::vector<QHash<InputEvent, std::vector<Bind>>> basic_maps;
    // The time (in ms) at which the timer is expired
    std::optional<uint64_t> current_timer;
    std::vector<std::pair<uint64_t, OutputEvent>> queued_events;

    std::atomic<bool> stopped = false;
    std::optional<std::function<void(QString)>> layer_changed_callback;
};
