#include "script_runner.h"
#include "temp_file_manager.h"
#include <QByteArray>
#include <QDebug>
#include <spawn.h>

extern char **environ;

TempFileManager temp_file_manager(".sh");

bool run_script(const QString &interpreter, const QString &script) {
    QString temp_path = temp_file_manager.get_temp_file(script, script);
    if (temp_path.isEmpty())
        return false;

    qDebug() << "Interpreter:" << interpreter;
    qDebug() << "Script path:" << temp_path;

    QByteArray interpreter_bytes = interpreter.toUtf8();
    QByteArray temp_path_bytes = temp_path.toUtf8();

    char *argv[] = {interpreter_bytes.data(), temp_path_bytes.data(), nullptr};

    posix_spawnattr_t attr;
    posix_spawnattr_init(&attr);

    pid_t pid;
    int status = posix_spawnp(&pid, argv[0], nullptr, &attr, argv, environ);
    posix_spawnattr_destroy(&attr);

    if (status != 0) {
        qDebug() << "Failed to spawn process:" << strerror(status);
        return false;
    }

    qDebug() << "Spawned script" << temp_path << "with pid" << pid;
    return true;
}
