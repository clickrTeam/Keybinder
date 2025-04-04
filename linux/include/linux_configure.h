#pragma once
#include <QString>

///
/// \brief Determines whether a user has a eventX device configured as their
/// keyboard in the config file
/// \param config_file_path: The path to the config file
/// \return QString eventX if an eventX device is found, empty string otherwise
///
QString retrieve_eventX(QString config_file_path);

///
/// \brief Determines whether a user has a eventX device configured as their
/// keyboard in the config file. This uses the default file path
/// \return QString eventX if an eventX device is found, empty string otherwise
///
QString retrieve_eventX();

///
/// \brief Stores the keyboard's eventX file in the config file
/// \param eventX_path: The string that represents which event device
/// corresponds to the keyboard
/// \return True if config file has successfully written, false otherwise
///
bool record_eventX(QString eventX_path);

///
/// \brief Prompts the user to press a key on their keyboard, then records which
/// device that press came from. Times out after so long to prevent users from
/// getting stuck.
/// \return The path to the eventX file corresponding to the keyboard.
///
QString detect_keyboard();
