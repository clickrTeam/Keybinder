#pragma once
#include "daemon.h"
#include "event.h"
#include "key_channel.h"
#include "profile.h"
#include "tray.h"
#include <QDebug>
#include <cstddef>
#include <mutex>
#include <optional>

using std::size_t;

class Mapper {
  public:
    Mapper(Profile, Daemon &, KeyReceiver);
    ~Mapper();
    void set_profile(Profile p);
    // This should be called in a seperate thread as it does not return
    void start();
    bool set_layer(size_t);

  private:
    void perform_binds(const QList<Bind> &bind);
    void set_layer_inner(size_t new_layer);

    std::mutex mtx;
    Daemon &daemon;
    KeyReceiver key_receiver;
    Profile profile;
    size_t cur_layer;
    Tray tray;

    QMap<KeyCode, Bind> key_press_triggers;
    QMap<KeyCode, Bind> key_release_triggers;
    QMap<KeyCode, std::pair<TapSequence, Bind>> tap_sequence_starts;

    // Sored the current tap sequence, the current key are we expected next
    // the next type we are expecting (up or down)
    std::optional<std::tuple<TapSequence, Bind, size_t, KeyEventType>>
        current_tap_sequence;
};
