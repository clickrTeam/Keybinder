#pragma once

#include <QString>

// Runs the given script with the specified interpreter.
// Uses the script content itself as the key for temp file caching.
// Returns true if the process was successfully spawned.
bool run_script(const QString &interpreter, const QString &script);
