#include <fstream>
#include <map>
#include <format>
#include <filesystem>
#include <string_view>
#include "settings.h"

using namespace Settings;

std::unique_ptr<Skin> Settings::skin;
float Settings::cursorSize = 1.0f;
int Settings::fieldSize = 20;
int Settings::roundTime = 60;
int Settings::colors = 10;
bool Settings::useCustomCursor = true;

std::map<std::string, std::string> Settings::ReadCfg(const std::string& path) {
    std::map<std::string, std::string> values;
    std::ifstream file(path);
    if (file.is_open()) {
        std::string line;
        std::string identifier;
        std::string value;
        auto trim = [](std::string& str, std::string_view t = " \t\n\r\f\v") -> std::string& {
            str.erase(str.find_last_not_of(t) + 1);
            str.erase(0, str.find_first_not_of(t));
            return str;
            };
        while (std::getline(file, line)) {
            auto index = line.find('=');
            if (index != std::string::npos) {
                identifier = line.substr(0, index);
                value = line.substr(index + 1);
                trim(identifier);
                trim(value);
                if (identifier != "" && value != "") {
                    values[identifier] = value;
                }
            }
        }
    }
    return values;
}

void Settings::WriteCfg(std::map<std::string, std::string> settings, const std::string& path) {
    std::ofstream file(path);
    if (file.is_open()) { 
        for (const auto& [parameter, value] : settings) {
            file << std::format("{} = {}\n", parameter, value);
        }
    }
}

void Settings::ReadSettingsFromMap(std::map<std::string, std::string>& values) {
	if (values.contains("skin")) {
		skin = TryLoadSkinOrDefaut(values["skin"]);
	}
	else {
		skin = std::make_unique<Skin>("Default");
	}
	if (values.contains("cursorSize")) {
		float value = std::stof(values["cursorSize"]);
		if (value >= 0.f && value <= 3.f) {
			cursorSize = value;
		}
	}
	if (values.contains("fieldSize")) {
		int value = std::stoi(values["fieldSize"]);
		if (value >= 1 && value <= 20) {
			fieldSize = value;
		}
	}
	if (values.contains("roundTime")) {
		int value = std::stoi(values["roundTime"]);
		if (value >= 1 && value <= 120) {
			roundTime = value;
		}
	}
	if (values.contains("colors")) {
		int value = std::stoi(values["colors"]);
		if (value >= 1 && value <= 10) {
			colors = value;
		}
	}
	if (values.contains("useCustomCursor")) {
		if (values["useCustomCursor"] == "true") {
			useCustomCursor = true;
		}
		else if (values["useCustomCursor"] == "false") {
			useCustomCursor = false;
		}
	}
}

std::map<std::string, std::string> Settings::SettingsToMap() {
	std::map<std::string, std::string> result;
	result["skin"] = skin->name;
	result["cursorSize"] = std::to_string(cursorSize);
	result["fieldSize"] = std::to_string(fieldSize);
	result["roundTime"] = std::to_string(roundTime);
	result["colors"] = std::to_string(colors);
	result["useCustomCursor"] = useCustomCursor?"true":"false";
	return result;
}

std::unique_ptr<Skin> Settings::TryLoadSkinOrDefaut(const std::string& skinName) {
	try {
		return TryLoadSkin(skinName);
	}
	catch (std::runtime_error& e){
		std::cout << e.what();
		std::cout << ", loading default skin instead\n";
		return std::make_unique<Skin>("Default");
	}
}

std::unique_ptr<Skin> Settings::TryLoadSkin(const std::string& skinName) {
	namespace fs = std::filesystem;
	const fs::path skin = "Skins/" + skinName;
	try {
		fs::current_path(skin);
	}
	catch (...) {
		std::string error = std::format("Could not open {} skin", skinName);
		fs::current_path("..\\..\\");
		throw std::runtime_error(error);
	}
	fs::current_path("..\\..\\");
	return std::make_unique<Skin>(skinName);
}

Settings::Skin::Skin(const std::string& skinName) : name{ skinName } {
	namespace fs = std::filesystem;
	const fs::path skin = "Skins/" + name;
	fs::current_path(skin);

	auto loadTexture = [](Texture2D& texture, const std::string& path) {
		texture = LoadTexture(path.c_str());
		if (texture.id <= 0) {
			std::cout << std::format("Failed to load {} - Loading file from default skin\n", path);
			std::string dPath = "../Default/" + path;
			texture = LoadTexture(dPath.c_str());
		}
		};

	loadTexture(background, "BackgroundImage.png");
	std::string tilePath;
	for (int i = 0; i < 10; i++) {
		tilePath = std::format("ColorTile{}.png", i);
		loadTexture(colorTiles[i], tilePath);
	}
	for (int i = 0; i < 10; i++) {
		tilePath = std::format("ColorTileRemoving{}.png", i);
		loadTexture(colorTilesRemoving[i], tilePath);
	}
	loadTexture(emptyTiles[0], "EmptyTile0.png");
	loadTexture(emptyTiles[1], "EmptyTile1.png");
	loadTexture(trail[0], "Trail0.png");
	loadTexture(trail[1], "Trail1.png");
	loadTexture(cursor, "cursor.png");

	auto loadSound = [](Sound& sound, const std::string& path) {
		sound = LoadSound(path.c_str());
		if (sound.frameCount <= 0) {
			std::cout << std::format("Failed to load {} - Loading file from default skin", path);
			std::string dPath = "../Default/" + path;
			sound = LoadSound(dPath.c_str());
		}
		};

	bool hitLoaded = false;
	bool missLoaded = false;
	for (const auto& dirEntry : fs::directory_iterator{ fs::current_path() }) {
		if (dirEntry.path().stem() == "hit") {
			std::string stringPath = dirEntry.path().string();
			loadSound(hitSound, stringPath);
			hitLoaded = true;
		}

		if (dirEntry.path().stem() == "miss") {
			std::string stringPath = dirEntry.path().string();
			loadSound(missSound, stringPath);
			missLoaded = true;
		}

		if (hitLoaded && missLoaded) {
			break;
		}
	}

	fs::current_path("..\\..\\");
	std::cout << "Skin loaded succesfuly\n";
}

Settings::Skin::~Skin() {
	UnloadSound(hitSound);
	UnloadSound(missSound);
	UnloadTexture(background);
	for (int i = 0; i < 10; i++) {
		UnloadTexture(colorTiles[i]);
	}
	for (int i = 0; i < 10; i++) {
		UnloadTexture(colorTilesRemoving[i]);
	}
	UnloadTexture(emptyTiles[0]);
	UnloadTexture(emptyTiles[1]);
	UnloadTexture(trail[0]);
	UnloadTexture(trail[1]);
	UnloadTexture(cursor);
}