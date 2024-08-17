#pragma once
#include <string>
#include <vector>
#include <optional>
#include "raymath.h"

struct ReplayTick {
    Vector2 mousePosition;
    bool isRelease;
};

using ReplayData = std::vector<ReplayTick>;

struct Score {
	int value{};
	int misscount{};
	float roundtime{};
	float misspenalty{};
	int fieldSize{};
	int colors{};
	std::string seed;
    ReplayData replayData;
    std::optional<int> replayFileOffset{};
    time_t date{};

    void Save(); 
};

Score LoadScore(const std::string& path); 
Score LoadScoreNoReplay(const std::string& path);
void LoadScoreReplay(Score& score, const std::string& path); 
