#pragma once
#include <string>

// Load config from path into Global::config.
// Safe to call before InitWindow.
// Returns true on success; on failure, leaves config at defaults.
bool LoadConfig(const std::string& path);

// Write current Global::config back to path.
bool SaveConfig(const std::string& path);
