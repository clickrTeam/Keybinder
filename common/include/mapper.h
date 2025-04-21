#pragma once
#include "event.h"
#include "profile.h"
#include <QDebug>
#include <cstddef>
#include <mutex>
#include <optional>

using std::size_t;

// Forward declaration needed due to circular dependency
class Daemon;

class Mapper {
  public:
    Mapper(Profile);
    ~Mapper();
    void set_profile(Profile p);
    void set_daemon(Daemon *d);
    bool map_input(InputEvent);
    bool set_layer(size_t);

  private:
    void perform_binds(const QList<Bind> &bind);
    void set_layer_inner(size_t new_layer);
    // void captureAndRelease();

    std::mutex mtx;
    Daemon *daemon = nullptr;
    Profile profile;
    size_t cur_layer;

    QMap<KeyCode, Bind> key_press_triggers;
    QMap<KeyCode, Bind> key_release_triggers;
    QMap<KeyCode, std::pair<TapSequence, Bind>> tap_sequence_starts;

    // Sored the current tap sequence, the current key are we expected next
    // the next type we are expecting (up or down)
    std::optional<std::tuple<TapSequence, Bind, size_t, KeyEventType>>
        current_tap_sequence;
};
