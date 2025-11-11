#pragma once

#include <QString>
#include <atomic>
#include <functional>
#include <thread>

class AppFocusListener {
  public:
    using Callback = std::function<void(const QString &)>;

    explicit AppFocusListener(Callback callback);

    void start();
    void stop();

    ~AppFocusListener();

  private:
    Callback callback;
    std::atomic<bool> stopFlag;
    std::thread listenerThread;

    void run();
};
