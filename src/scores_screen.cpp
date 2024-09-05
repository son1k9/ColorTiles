#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <exception>
#include <filesystem>
#include <fstream>
#include <format>
#include <utility>
#include "raylib.h"
#include "raygui.h"
#include "screen.h"
#include "score.h"

ScoresScreen::ScoresScreen() {
    namespace fs = std::filesystem;

    fs::path replaysFolder = "Replays";
    if (!fs::exists(replaysFolder)) {
        return;
    }

    for (const auto& dirEntry : fs::directory_iterator{ replaysFolder }) {
        const auto& path = dirEntry.path();
        const auto& extension = path.extension();
        if (extension == ".ctr") {
            try {
                scores.push_back(std::make_pair<Score, std::string>(LoadScoreNoReplay(path.string()), path.string()));
            }
            catch (std::exception& e) {
                std::cout << e.what();
            }
        }
    }
}

void ScoresScreen::Update() {
    if (IsKeyPressed(KEY_ESCAPE)) {
        RequestScreenChange(Screens::MainMenu);
    }
}

void ScoresScreen::Draw() {
    constexpr auto fontsize = 20;
    constexpr auto gap = 5;
    constexpr auto marginLeft = 5;

    GuiSetStyle(DEFAULT, TEXT_SIZE, fontsize);
    GuiSetStyle(DEFAULT, BORDER_WIDTH, 1);

    int width = Rayutils::GetDisplayWidth();
    int height = Rayutils::GetDisplayHeight();

    Rectangle panelRec = { (width - 800) / 2, (height - 600) / 2, 800, 600 };
    Rectangle panelContentRec = { (width - 800) / 2 + 2, (height - 600) / 2, 800 - 4, 600 };

    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    panelContentRec.height = fontsize * scores.size();
    GuiScrollPanel(panelRec, "Scores", panelContentRec, &panelScroll, &panelView);

    int count = 1;
    for (auto& [score, path] : scores) {
        std::tm* time = std::localtime(&score.date);
        std::ostringstream oss;
        oss << std::put_time(time, "%d.%m.%Y %H:%M:%S");
        const std::string scoreStr = oss.str() + std::format(" {}", score.value);
        const Rectangle scoreLabel{ panelRec.x + panelScroll.x + marginLeft,
            panelRec.y + panelScroll.y + count * (fontsize + gap),
            panelRec.x + panelScroll.x + marginLeft + 300,
            fontsize };
        GuiLabel(scoreLabel, scoreStr.c_str());
        if (GuiButton({ panelRec.x + panelScroll.x + marginLeft + 300, panelRec.y + panelScroll.y + count * (fontsize + gap), static_cast<float>(MeasureText("Replay", 20)), fontsize + 2 }, "Replay")) {
            std::cout << "Clicked score: " << scoreStr << "\n";
            LoadScoreReplay(score, path);
            RequestReplay(std::move(score));
        }
        count++;
    }
}


