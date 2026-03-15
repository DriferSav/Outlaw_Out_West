#include <config.hpp>
#include <globals.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
using json = nlohmann::json;

bool LoadConfig(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return false;
    try {
        json d = json::parse(f);
        auto& c = Global::config;
        if (d.contains("audio")) {
            c.masterVolume = d["audio"].value("master_volume", 1.0f);
            c.musicVolume  = d["audio"].value("music_volume",  0.7f);
            c.sfxVolume    = d["audio"].value("sfx_volume",    1.0f);
        }
        return true;
    } catch (...) { return false; }
}

bool SaveConfig(const std::string& path) {
    auto& c = Global::config;
    json d = { {"audio", {
        {"master_volume", c.masterVolume},
        {"music_volume",  c.musicVolume},
        {"sfx_volume",    c.sfxVolume}
    }}};
    std::ofstream f(path);
    if (!f.is_open()) return false;
    f << d.dump(2);
    return true;
}
