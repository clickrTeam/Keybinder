#pragma once
#include "daemon.h"
#include "event.h"
#include "profile.h"
#include <QDebug>
#include <cstddef>

using std::size_t;

class Mapper {
  public:
    Mapper(Profile &);
    ~Mapper();
    void set_daemon(Daemon *d);
    bool mapInput(InputEvent);

  private:
    void captureAndRelease();
    Daemon *daemon = nullptr;
    Profile &profile;
    size_t cur_layer;
    QMap<int, int>
        timedKeyProgress; // represents progress towards finish the timed macro.
    int next_key = -1; // the next expected key in a timed macro. Never includes
                       // the first.
    int first_key = -1; // the first key in a expected macro.

    bool thenRelease = false; // true if some key is captured to release.
    int capture_and_release_key = -1; // the key captured to release.
};
