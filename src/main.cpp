#include <filesystem>
#include <fstream>
#include <map>
#include "raylib.h"
#include "settings.h"
#include "screen.h"

static void ChangeScreen();
static void ProcessGlobalInput();
void PlayReplay();

static int lastWidth{};
static int lastHeight{};
static std::unique_ptr<Screen> currentScreen;

bool close = false;
bool changeScreen = false;
Screens changeTo;

bool playReplay = false;
Score replay;

int main()
{
    InitWindow(1600, 900, "Color Tiles");
    InitAudioDevice();
    SetMasterVolume(0.5f);
    SetTargetFPS(2 * GetMonitorRefreshRate(GetCurrentMonitor()));
    SetExitKey(-1);

    auto m = Settings::ReadCfg("settings.cfg");
    Settings::ReadSettingsFromMap(m);

    currentScreen = std::make_unique<MainMenuScreen>();

    while (!WindowShouldClose() && !close)
    {
        if (changeScreen) {
            ChangeScreen();
        }
        else if (playReplay) {
            PlayReplay();
        }

        ProcessGlobalInput();

        currentScreen->Update();

        BeginDrawing();

        currentScreen->Draw();

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();

    return 0;
}

void PlayReplay() {
    auto screen = std::make_unique<GameplayScreen>(Settings::fieldSize, Settings::roundTime, Settings::colors);
    screen->PlayReplay(std::move(replay));
    currentScreen = std::move(screen);
    playReplay = false;
}

void ChangeScreen() {
    switch (changeTo)
    {
    case Screens::MainMenu:
        currentScreen = std::make_unique<MainMenuScreen>();
        break;
    case Screens::Gameplay:
        currentScreen = std::make_unique<GameplayScreen>(Settings::fieldSize, Settings::roundTime, Settings::colors);
        break;
    case Screens::Settings:
        currentScreen = std::make_unique<SettingsScreen>();
        break;
    case Screens::Scores:
        currentScreen = std::make_unique<ScoresScreen>();
        break;
    default:
        break;
    }
    changeScreen = false;
}

void ProcessGlobalInput() {
    if (IsKeyPressed(KEY_F11)) {
        if (!IsWindowFullscreen())
        {
            lastWidth = GetScreenWidth();
            lastHeight = GetScreenHeight();

            int monitor = GetCurrentMonitor();
            int mWidth = GetMonitorWidth(monitor);
            int mHeight = GetMonitorHeight(monitor);

            SetWindowSize(mWidth, mHeight);
            ToggleFullscreen();
        }
        else {
            ToggleFullscreen();
            SetWindowSize(lastWidth, lastHeight);
        }
    }
}