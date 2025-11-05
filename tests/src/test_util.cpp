#include "test_util.h"
#include "daemon.h"
#include "key_channel.h"
#include "key_counter.h"
#include "mapper.h"
#include "settings.h"
#include "util.h"
#include <QThread>
#include <chrono>
#include <gtest/gtest.h>
#include <memory>

void mapper_test(Profile p, QList<InputEvent> events,
                 QList<OutputEvent> outputs) {
    QList<uint64_t> waits(events.size(), 0);
    mapper_test_timed(p, events, waits, outputs);
}

void mapper_test_timed(Profile p, QList<InputEvent> events,
                       QList<uint64_t> waits, QList<OutputEvent> outputs) {
    ASSERT_EQ(events.size(), waits.size());
    auto [rx, tx] = create_channel();
    Daemon d(rx);

    KeybinderSettings settings;
    KeyCounter key_counter;
    auto m = std::make_shared<Mapper>(p, d, tx, settings, key_counter);
    QThread *mapper_thread = QThread::create([m] { m->start(); });
    mapper_thread->start();
    uint64_t time = current_time_ms();
    size_t i = 0;
    while (i < events.size() || m->processed_events() < events.size() ||
           m->waiting_for_timer()) {

        if (i < events.size()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(waits[i]));
            rx.send_key(events[i]);
            i++;
        }
    }
    ASSERT_EQ(d.get_outputs(), outputs);
    m->stop();
    mapper_thread->wait();
}
