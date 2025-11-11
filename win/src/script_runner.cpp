// run_script_helper.cpp
#include "script_runner.h"
#include "temp_file_manager.h"
#include <QDebug>
#include <string>
#include <vector>
#include <windows.h>

bool run_script_bind(const RunScriptBind &bind) {
    // Determine extension only if needed
    QString ext;
    QString interp_lower = bind.interpreter.toLower();
    if (interp_lower.contains("powershell") || interp_lower == "pwsh")
        ext = ".ps1";
    else if (interp_lower.contains("cmd"))
        ext = ".bat";
    else
        ext = ""; // Python, Rust, Bash etc. → no extension needed

    // Use static TempFileManager; no need to reconstruct per call
    static TempFileManager tmp_mgr;

    QString temp_path = tmp_mgr.get_temp_file(bind.script, bind.script, ext);
    if (temp_path.isEmpty()) {
        qWarning() << "run_script_bind: failed to create temp file";
        return false;
    }

    // Build command line based on known extension
    QString command_line;
    if (ext == ".bat") {
        command_line = QStringLiteral("cmd.exe /C \"%1\"").arg(temp_path);
    } else if (ext == ".ps1") {
        command_line =
            QStringLiteral(
                "powershell.exe -ExecutionPolicy Bypass -File \"%1\"")
                .arg(temp_path);
    } else {
        command_line =
            QStringLiteral("\"%1\" \"%2\"").arg(bind.interpreter, temp_path);
    }

    // Prepare mutable C string for CreateProcessA
    std::string cmd_std = command_line.toStdString();
    std::vector<char> cmd_buf(cmd_std.begin(), cmd_std.end());
    cmd_buf.push_back('\0');

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    if (!CreateProcessA(nullptr, cmd_buf.data(), nullptr, nullptr, FALSE, 0,
                        nullptr, nullptr, &si, &pi)) {
        qWarning() << "run_script_bind: CreateProcess failed. Error:"
                   << GetLastError();
        return false;
    }

    // Fire-and-forget: do NOT wait, just close handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true; // process launched successfully
}
