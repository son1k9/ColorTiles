#define _CRT_SECURE_NO_WARNINGS
#define RAYGUI_IMPLEMENTATION

#include <algorithm>
#include <iostream>
#include <random>
#include <map>
#include <format>
#include <filesystem>
#include <fstream>
#include <chrono>
#include "raylib.h"
#include "raymath.h"
#include "raygui.h"
#include "rayutils.h"
#include "settings.h"
#include "score.h"
#include "screen.h"
#include "random.h"

using namespace Rayutils;

GameplayScreen::GameplayScreen(int fieldSize, float roundtime, int colors, float misspenalty) :field(fieldSize* fieldSize), roundtime{ roundtime }, misspenalty{ misspenalty }, fieldSize{ fieldSize },
currentSeed(seedLenght, 0), timer{ roundtime }, colors{ colors } {
    if (Settings::useCustomCursor) {
        HideCursor();
    }

    Reset();
}

void GameplayScreen::PlayReplay(Score&& p_replay) {
    this->replay = std::move(p_replay);
    fieldSize = replay.fieldSize;
    colors = replay.colors;
    currentSeed = replay.seed;
    roundtime = replay.roundtime;
    misspenalty = replay.misspenalty;
    replayMode = true;
    timer = Timer{ roundtime };
    regenerateOnReset = false;
    strcpy(seedInputBuffer, currentSeed.c_str());
    ShowCursor();
    Reset();
}

void GameplayScreen::Update() {
    timer.Update();
    bool mouseRelease = false;

    const int height = GetDisplayHeight();
    const int width = GetDisplayWidth();

    if (IsKeyPressed(KEY_ESCAPE)) {
        ShowCursor();
        RequestScreenChange(Screens::MainMenu);
    }

    constexpr auto ticktime = 1.0 / tickrate;
    if (!replayMode) {
        mousePosition = GetMousePosition();
        mouseRelease = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

        if (!seedEditMod) {
            if (IsKeyPressed(KEY_R)) {
                Reset();
            }
        }
    }
    else if ((currentTickIndex < replay.replayData.size()) && (GetTime() - resetTime > currentTickIndex * ticktime)) {
        const auto& tick = replay.replayData[currentTickIndex];
        mousePosition = { tick.mousePosition.x * width, tick.mousePosition.y * height};
        mouseRelease = tick.isRelease;
        currentTickIndex++;
    }

    if (mouseRelease) {
        currentTick.isRelease = true;
        auto fieldPosition = WorldToFieldPosition(mousePosition);
        if (fieldPosition.x != -1 && !IsTimeEnded()) {
            FieldRelease(fieldPosition);
        }
    }

    if (!replayMode && !IsTimeEnded() && (GetTime() - resetTime > score.replayData.size() * ticktime)) {
        currentTick = { {mousePosition.x / width, mousePosition.y / height}, currentTick.isRelease };
        score.replayData.push_back(currentTick);
        currentTick.isRelease = false;
    }

    const float delta = GetFrameTime();
    for (auto& [_, data] : removingTilesPosition) {
        if ((data.position.y * tileSize + fieldTopLeft.y) > height) {
            continue;
        }
        constexpr Vector2 a{ 0, 75 };
        data.position = Vector2Add(data.position, Vector2Add(Vector2Scale(data.velocity, delta), Vector2Scale(a, delta * delta / 2)));
        data.velocity = Vector2Add(data.velocity, Vector2Scale(a, delta));
    }

    for (auto& [_, data] : trailColor) {
        if (data.color.a <= 0) {
            continue;
        }
        const float colorDiff = delta * 500.0f;
        data.alpha += colorDiff;
        if (255 - data.alpha < 0) {
            data.color.a = 0;
        }
        else {
            data.color.a = 255 - data.alpha;
        }
    }

    if (!replayMode && IsTimeEnded() && !scoreSaved) {
        namespace chr = std::chrono;
        auto currentTime = chr::system_clock::to_time_t(chr::system_clock::now());
        score.date = currentTime;
        score.Save();
        scoreSaved = true;
    }
}

void GameplayScreen::Draw() {
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    GuiSetStyle(DEFAULT, BORDER_WIDTH, 2);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(BLACK));
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, ColorToInt(BLACK));

    const Settings::Skin& skin = *Settings::skin;
    width = GetDisplayWidth();
    height = GetDisplayHeight();
    constexpr int padding = 20;
    tileSize = (height - padding * 2) / fieldSize;
    const int error = (int)(fieldSize * ((float)(height - padding * 2) / fieldSize - tileSize));

    fieldTopLeft.x = (width - fieldSize * tileSize) / 2;
    fieldTopLeft.y = padding + error / 2;
    fieldBottomRight.x = fieldSize * tileSize + fieldTopLeft.x;
    fieldBottomRight.y = fieldSize * tileSize + fieldTopLeft.y;

    //Background
    ClearBackground(RAYWHITE);
    DrawFullTexture(skin.Background(), { .x = 0, .y = 0, .width = (float)width, .height = (float)height });

    //Border
    constexpr int borderThickness = 3;
    Rectangle border{ fieldTopLeft.x - borderThickness, fieldTopLeft.y - borderThickness, fieldBottomRight.x - fieldTopLeft.x + 2 * borderThickness, fieldBottomRight.y - fieldTopLeft.y + 2 * borderThickness };
    DrawRectangleLinesEx(border, 3, BLACK);

    //Timer
    const int spaceRight = width - fieldBottomRight.x;
    const int timeTextWidth = 0.1 * spaceRight;
    const int timeBarWidth = 0.8 * spaceRight;
    constexpr int timebarHeight = 20;
    Rectangle timeBar{ 0.025 * spaceRight, fieldTopLeft.y - borderThickness, (timer.TimeLeft() / roundtime) * timeBarWidth, timebarHeight };
    DrawRectangleRec(timeBar, RED);
    timeBar.width = timeBarWidth;
    DrawRectangleLinesEx(timeBar, 2, BLACK);
    DrawText(std::to_string((int)round(timer.TimeLeft())).c_str(), 0.025 * spaceRight + timeBarWidth + 5, fieldTopLeft.y - borderThickness, 20, RED);

    //Seed
    DrawText("Current seed:", 0.025 * spaceRight, (float)fieldTopLeft.y - borderThickness + timebarHeight + 20, 20, BLACK);
    if (GuiTextBox(Rectangle{ 0.025f * spaceRight, (float)fieldTopLeft.y - borderThickness + timebarHeight + 40, (float)timeBarWidth - 120, 30 }, seedInputBuffer, sizeof(seedInputBuffer), seedEditMod && !replayMode)) {
        if (!seedEditMod) {
            regenerateOnReset = false;
        }
        seedEditMod = !seedEditMod;

        ValidateSeed();
    }
    if (GuiButton(Rectangle{ 0.025f * spaceRight + timeBarWidth - 120 + 5, (float)fieldTopLeft.y - borderThickness + timebarHeight + 40, (float)MeasureText("Copy", 30), 30 }, "Copy")) {
        SetClipboardText(currentSeed.c_str());
    }
    if (!replayMode) {
        if (GuiButton(Rectangle{ 0.025f * spaceRight + timeBarWidth - 120 + (float)MeasureText("Copy", 30) + 10, (float)fieldTopLeft.y - borderThickness + timebarHeight + 40, (float)MeasureText("Paste", 30), 30 }, "Paste")) {
            if (!seedEditMod) {
                regenerateOnReset = false;
            }

            const char* clipboardText = GetClipboardText();
            char buffer[seedLenght + 1]{ 0 };
            for (int i = 0; clipboardText[i] != '\0' && i < sizeof(buffer) - 1; i++) {
                buffer[i] = clipboardText[i];
            }
            strcpy(seedInputBuffer, buffer);

            ValidateSeed();
        }
        GuiCheckBox(Rectangle{ 0.025f * spaceRight, (float)fieldTopLeft.y - borderThickness + timebarHeight + 70 + 10, 20, 20 }, "New seed on reset", &regenerateOnReset);
    }

    //Score
    std::string scoreString = std::format("Score: {}", score.value);
    DrawText(scoreString.c_str(), width - MeasureText(scoreString.c_str(), 40) - 0.1 * spaceRight, fieldTopLeft.y - borderThickness, 40, BLACK);
    std::string misscountString = std::format("Miss cound: {}", score.misscount);
    DrawText(misscountString.c_str(), width - MeasureText(scoreString.c_str(), 40) - 0.1 * spaceRight, fieldTopLeft.y - borderThickness + 40, 40, BLACK);

    auto isEven = [](int x) -> bool {
        return (x & 1) == 0;
        };

    //Field background
    for (int y = 0; y < fieldSize; y++)
        for (int x = 0; x < fieldSize; x++) {
            int index = isEven(x) xor isEven(y);
            Rectangle rect{ x * tileSize + fieldTopLeft.x, y * tileSize + fieldTopLeft.y, tileSize, tileSize };
            DrawFullTexture(skin.EmptyTile(index), rect);
        }

    //Color Tiles
    for (int i = 0; i < field.size(); i++) {
        const auto& tile = field[i];
        if (tile.color != 0 && tile.isActive) {
            const auto [x, y] = Index1DTo2D(i);
            DrawTexturePro(
                skin.ColorTile(tile.color - 1),
                { .x = 0, .y = 0, .width = (float)skin.ColorTile(tile.color - 1).width, .height = (float)skin.ColorTile(tile.color - 1).height },
                { .x = (float)x * tileSize + fieldTopLeft.x, .y = (float)y * tileSize + fieldTopLeft.y, .width = (float)tileSize, .height = (float)tileSize },
                { .x = 0, .y = 0 },
                0,
                WHITE
            );
        }
    }

    for (auto& [i, data] : trailColor) {
        const auto [x, y] = Index1DTo2D(i);
        if (data.color.a != 0) {
            const int index = isEven(x) xor isEven(y);
            DrawTexturePro(
                skin.Trail(index),
                { .x = 0, .y = 0, .width = (float)skin.Trail(index).width, .height = (float)skin.Trail(index).height },
                { .x = (float)x * tileSize + fieldTopLeft.x, .y = (float)y * tileSize + fieldTopLeft.y, .width = (float)tileSize, .height = (float)tileSize },
                { .x = 0, .y = 0 },
                0,
                { .r = 255, .g = 255, .b = 255, .a = data.color.a }
            );
        }
    }

    for (auto& [i, data] : removingTilesPosition) {
        if ((data.position.y * tileSize + fieldTopLeft.y) > height) {
            continue;
        }
        const auto& tile = field[i];
        const auto [x, y] = Index1DTo2D(i);
        DrawTexturePro(
            skin.ColorTileRemoving(tile.color - 1),
            { .x = 0, .y = 0, .width = (float)skin.ColorTileRemoving(tile.color - 1).width, .height = (float)skin.ColorTileRemoving(tile.color - 1).height },
            { .x = data.position.x * tileSize + fieldTopLeft.x, .y = data.position.y * tileSize + fieldTopLeft.y, .width = (float)tileSize, .height = (float)tileSize },
            { .x = 0, .y = 0 },
            0,
            WHITE
        );
    }

    if (IsTimeEnded()) {
        DrawOutlinedText(scoreString.c_str(), (width - MeasureText(scoreString.c_str(), 150)) / 2, (height - 150) / 2, 150, WHITE, 5, BLACK);
    }

    //Cursor
    if (Settings::useCustomCursor || replayMode) {
        const auto mPosition = mousePosition;
        const float cursorSize = Settings::cursorSize;
        DrawFullTexture(
            Settings::skin->Cursor(),
            { .x = mPosition.x, .y = mPosition.y, .width = skin.Cursor().width * cursorSize, .height = skin.Cursor().height * cursorSize },
            { .x = skin.Cursor().width * cursorSize / 2, .y = skin.Cursor().height * cursorSize / 2 });
    }
}

std::string GameplayScreen::GenerateSeed() {
    std::string result(seedLenght, 0);
    const auto randchar = [&]() {
        return seedSymbols[Random::geti(0, 61)];
        };
    std::generate_n(result.begin(), seedLenght, randchar);
    return result;
}

void GameplayScreen::Reset() {
    if (!replayMode) {
        scoreSaved = false;
        score.replayData.clear();
    }

    if (regenerateOnReset) {
        currentSeed = GenerateSeed();
        std::cout << "Generated seed: " << currentSeed << '\n';
        strcpy(seedInputBuffer, currentSeed.c_str());
    }

    auto seed = std::hash<std::string>{}(currentSeed);
    Random::seed(seed);

    removingTilesPosition.clear();
    trailColor.clear();
    std::uniform_int_distribution<int> dist{ 0, 2 }; 	//The chance for a color tile is 66% 
    std::uniform_int_distribution<int> colorDist{ 1, colors };
    for (int i = 0; i < field.size(); i++) {
        int isColor = dist(Random::mt);
        if (!isColor) {
            field[i].color = 0;
            field[i].isActive = false;
        }
        else {
            field[i].color = colorDist(Random::mt);
            field[i].isActive = true;
        }
    }

    score = { .roundtime = roundtime, .misspenalty = misspenalty, .fieldSize = fieldSize, .colors = colors, .seed = currentSeed };
    RestartTimers();
}

void GameplayScreen::RestartTimers() {
    timer.Start();
    resetTime = GetTime();
}

void GameplayScreen::Miss() {
    PlaySound(Settings::skin->MissSound());
    score.misscount++;
    timer.SubtractTime(misspenalty);
    std::cout << "Miss\n";
}

void GameplayScreen::FieldRelease(Vector2i position) {
    if (field[Index2DTo1D(position.x, position.y)].isActive) {
        std::cout << "Release on colored tile\n";
        Miss();
        return;
    }
    const auto tiles = FindColorTiles(position);
    const int oldScore = score.value;
    if (!RemoveTiles(position, tiles)) {
        Miss();
        return;
    }
    std::cout << "Hit. Score gained: " << score.value - oldScore << '\n';
    PlaySound(Settings::skin->HitSound());
}

std::map<int, std::vector<int>> GameplayScreen::FindColorTiles(Vector2i position) const {
    std::map<int, std::vector<int>> result;

    //Check left
    for (int i = position.x - 1; i >= 0; i--) {
        const int index = Index2DTo1D(i, position.y);
        const auto& tile = field[index];
        if (tile.isActive) {
            result[tile.color].push_back(index);
            break;
        }
    }
    //Check top
    for (int i = position.y - 1; i >= 0; i--) {
        const int index = Index2DTo1D(position.x, i);
        const auto& tile = field[index];
        if (tile.isActive) {
            result[tile.color].push_back(index);
            break;
        }
    }
    //Check right
    for (int i = position.x + 1; i < fieldSize; i++) {
        const int index = Index2DTo1D(i, position.y);
        const auto& tile = field[index];
        if (tile.isActive) {
            result[tile.color].push_back(index);
            break;
        }
    }
    //Check bottom
    for (int i = position.y + 1; i < fieldSize; i++) {
        const int index = Index2DTo1D(position.x, i);
        const auto& tile = field[index];
        if (tile.isActive) {
            result[tile.color].push_back(index);
            break;
        }
    }

    return result;
}

bool GameplayScreen::RemoveTiles(Vector2i clickPosition, const std::map<int, std::vector<int>>& colorTiles) {
    bool removed = false;

    for (const auto& [color, tiles] : colorTiles) {
        if (tiles.size() >= 2) {
            removed = true;
            int count = 0;
            for (const auto& tileIndex : tiles) {
                auto& tile = field[tileIndex];
                tile.isActive = false;
                StartRemoveLineAnim(clickPosition, Index1DTo2D(tileIndex));
                count++;
            }
            score.value += (int)pow(2, count - 1);
        }
    }

    return removed;
}

void GameplayScreen::StartRemoveLineAnim(Vector2i startPosition, Vector2i removedTilePosition) {
    const auto step = Vector2i::CompareUnit(startPosition, removedTilePosition);
    auto i = startPosition;
    int tileIndex{};
    for (; i != removedTilePosition; i = i + step) {
        tileIndex = Index2DTo1D(i.x, i.y);
        trailColor[tileIndex].color.a = 255;
        trailColor[tileIndex].alpha = 0;
    }
    tileIndex = Index2DTo1D(i.x, i.y);
    trailColor[tileIndex].color.a = 255;
    std::uniform_real_distribution<float> removedTileSpeedX{ -4, 4 };
    std::uniform_real_distribution<float> removedTileSpeedY{ -14.0 , -8.0 };
    removingTilesPosition[tileIndex].velocity = Vector2{ removedTileSpeedX(Random::mt), removedTileSpeedY(Random::mt) };
    removingTilesPosition[tileIndex].position = { (float)i.x, (float)i.y };
}

bool GameplayScreen::ValidateSeed() {
    bool valid = true;
    for (int i = 0; i < sizeof(seedInputBuffer) - 1; i++) {
        bool cvalid = false;
        for (int j = 0; j < sizeof(seedSymbols) - 1; j++) {
            if (seedInputBuffer[i] == seedSymbols[j]) {
                cvalid = true;
            }
        }
        if (!cvalid) {
            valid = false;
            break;
        }
    }

    if (valid) {
        currentSeed = std::string(seedInputBuffer);
    }
    else {
        strcpy(seedInputBuffer, currentSeed.c_str());
    }
    return valid;
}