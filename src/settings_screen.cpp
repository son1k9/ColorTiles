#include <string>
#include <filesystem>
#include "raylib.h"
#include "raygui.h"
#include "rayutils.h"
#include "screen.h"
#include "settings.h"

SettingsScreen::SettingsScreen() {
    namespace fs = std::filesystem;

    for (const auto& dirEntry : fs::directory_iterator{ "Skins" }) {
        if (dirEntry.is_directory()) {
            skins.push_back(dirEntry.path().filename().string());
        }
    }

    for (int i = 0; i < skins.size(); i++) {
        if (Settings::skin->name == skins[i]) {
            skinActive = i;
            previousSkin = i;
        }
        skinsStr += skins[i];
        if ((i + 1) < skins.size()) {
            skinsStr += ";";
        }
    }
}

void SettingsScreen::Update() {
    if (IsKeyPressed(KEY_ESCAPE)) {
        Settings::WriteCfg(Settings::SettingsToMap(), "settings.cfg");
        RequestScreenChange(Screens::MainMenu);
    }
}

void SettingsScreen::Draw() {
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    GuiSetStyle(DEFAULT, BORDER_WIDTH, 1);

    int width = Rayutils::GetDisplayWidth();
    int height = Rayutils::GetDisplayHeight();

    Vector2 leftTopCorner{ (width - 800) / 2, (height - 600) / 2 };

    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    if (skinEditMode) GuiLock();

    GuiGroupBox({ .x = leftTopCorner.x, .y = leftTopCorner.y, .width = 800, .height = 600 }, "Settings");

    GuiLabel({ .x = leftTopCorner.x + 24, .y = leftTopCorner.y + 64, .width = 150, .height = 24 }, "Cursor size");
    GuiSliderBar({ .x = leftTopCorner.x + 184, .y = leftTopCorner.y + 64, .width = 120, .height = 24 }, "0", "3", &Settings::cursorSize, 0, 3);
    std::string cursorSizeStr = std::to_string(Settings::cursorSize);
    GuiLabel({ .x = leftTopCorner.x + 334, .y = leftTopCorner.y + 64, .width = 120, .height = 24 }, cursorSizeStr.c_str());

    GuiLabel({ .x = leftTopCorner.x + 24, .y = leftTopCorner.y + 24, .width = 150, .height = 24 }, "Skin");
    if (GuiDropdownBox({ .x = leftTopCorner.x + 184, .y = leftTopCorner.y + 24, .width = 120, .height = 24 }, skinsStr.c_str(), &skinActive, skinEditMode)) {
        if (skinEditMode && previousSkin != skinActive) {
            try {
                Settings::skin = Settings::TryLoadSkin(skins[skinActive]);
                previousSkin = skinActive;
            }
            catch (std::runtime_error& e) {
                std::cout << e.what();
                skinActive = previousSkin;
            }
        }
        skinEditMode = !skinEditMode;
    }
    GuiCheckBox({ .x = leftTopCorner.x + 24, .y = leftTopCorner.y + 104, .width = 24, .height = 24 }, "Use skin's cursor", &Settings::useCustomCursor);

    GuiUnlock();
}