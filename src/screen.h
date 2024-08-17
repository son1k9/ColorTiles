#pragma once

#include <vector>
#include <random>
#include <map>
#include <set>
#include <utility>
#include "raylib.h"
#include "rayutils.h"
#include "score.h"

class Screen {
public:
    virtual void Update() = 0;
    virtual void Draw() = 0;
    virtual ~Screen() = default;
};

enum class Screens {
    MainMenu,
    Gameplay,
    Scores,
    Settings
};

void RequestScreenChange(Screens screen);
void RequestReplay(Score&& score);

class GameplayScreen : public Screen {
public:
    GameplayScreen(int fieldSize, float roundtime = 60.0, int colors = 10, float misspenalty = 5.0);
    void Update() override;
    void Draw() override;
    void PlayReplay(Score&& score);
private:
    struct Tile {
        int color{};
        bool isActive{};

        struct PositionData {
            Vector2 position{};
            Vector2 velocity{};
        };

        struct TrailColor {
            Color color{ 200, 200, 200, 255 };
            float alpha = 0;
        };
    };

    class Timer {
    private:
        float timeLeft;
        bool active = false;
    public:
        float durationSec;

        Timer(float durationSec) : durationSec{ durationSec } {
            timeLeft = durationSec;
        }

        //If timer is running restarts it
        void Start() {
            timeLeft = durationSec;
            active = true;
        }
        float TimeLeft() const {
            return timeLeft >= 0 ? timeLeft : 0;
        }
        void Update() {
            if (active) {
                if (timeLeft > 0) {
                    timeLeft -= GetFrameTime();
                }
                else {
                    active = false;
                }
            }
        }
        void SubtractTime(float seconds) {
            if (active && timeLeft > 0) {
                timeLeft -= seconds;
            }
        }
    };

    int Index2DTo1D(int x, int y) const {
        return y * fieldSize + x;
    }

    Rayutils::Vector2i Index1DTo2D(int index) const {
        const int x = index % fieldSize;
        const int y = index / fieldSize;
        return { x, y };
    }

    bool IsTimeEnded() const {
        return timer.TimeLeft() <= 0;
    }

    Rayutils::Vector2i WorldToFieldPosition(Vector2 position) const {
        if (position.x < fieldTopLeft.x || position.x > fieldBottomRight.x || position.y < fieldTopLeft.y || position.y > fieldBottomRight.y) {
            return { -1, -1 };
        }
        const int x = (int)(position.x - fieldTopLeft.x) / tileSize;
        const int y = (int)(position.y - fieldTopLeft.y) / tileSize;
        return { x, y };
    }

    void FieldRelease(Rayutils::Vector2i position);
    std::map<int, std::vector<int>> FindColorTiles(Rayutils::Vector2i position) const;
    bool RemoveTiles(Rayutils::Vector2i clickPosition, const std::map<int, std::vector<int>>& tiles);
    void StartRemoveLineAnim(Rayutils::Vector2i startPosition, Rayutils::Vector2i removedTilePosition);
    bool ValidateSeed();
    void Reset();
    void Miss();
    std::string GenerateSeed();
    void RestartTimers();

    //Field
    int fieldSize{};
    std::vector<Tile> field;
    int tileSize{};
    int width{};
    int height{};
    Vector2 mousePosition{};
    Rayutils::Vector2i fieldTopLeft{};
    Rayutils::Vector2i fieldBottomRight{};
    int colors{};

    //Removing animation
    std::map<int, Tile::PositionData> removingTilesPosition;
    std::map<int, Tile::TrailColor> trailColor;

    //Score
    Score score{};
    bool scoreSaved{ false };
    static const auto tickrate = 100;
    ReplayTick currentTick{};
    double resetTime;

    //Time
    float roundtime{};
    float misspenalty{};
    Timer timer;

    //Seed generation
    const char seedSymbols[62] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
                            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
    std::string currentSeed;
    static const int seedLenght = 10;
    bool regenerateOnReset{ true };

    //UI
    char seedInputBuffer[seedLenght + 1]{ 0 };
    bool seedEditMod{ false };

    //Replay
    bool replayMode{ false };
    Score replay{};
    size_t currentTickIndex{0};
};

class SettingsScreen : public Screen {
private:
    bool skinEditMode = false;
    int previousSkin = 0;
    int skinActive = 0;
    std::vector<std::string> skins;
    std::string skinsStr;
public:
    SettingsScreen();
    void Update() override;
    void Draw() override;
};

class MainMenuScreen : public Screen {
private:
    bool showDialogWindow = false;
    bool fieldSizeEdit = false;
    bool colorsEdit = false;
    bool roundTimeEdit = false;

    void Quit();
public:
    MainMenuScreen();
    void Update() override;
    void Draw() override;
};

class ScoresScreen : public Screen {
private:
    Rectangle panelView = {};
    Vector2 panelScroll = {};
    bool showContentArea = true;
    std::vector<std::pair<Score, std::string>> scores;
public:
    ScoresScreen();
    void Update() override;
    void Draw() override;
};