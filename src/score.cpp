#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <format>
#include <chrono>
#include "score.h"

void Score::Save() {
    namespace fs = std::filesystem;
    fs::path replaysFolder = "Replays";
    if (!fs::exists(replaysFolder)){
        fs::create_directory(replaysFolder);
    }
    
    auto toStringPadLeft = [](auto value, int minLenght) -> std::string {
        std::string valueStr = std::to_string(value);
        int lenghtDiff = minLenght - valueStr.length();
        if (lenghtDiff <= 0){
            return valueStr;
        }
        std::string result(minLenght - valueStr.length(), '0');
        result += valueStr; 
        return result; 
    };

    std::string hashString;
    hashString += toStringPadLeft(value, 5);
    hashString += toStringPadLeft(misscount, 5);
    hashString += toStringPadLeft(roundtime, 5);
    hashString += toStringPadLeft(misspenalty, 5);
    hashString += toStringPadLeft(fieldSize, 5);
    hashString += toStringPadLeft(colors, 5);
    hashString += seed;
    hashString += std::ctime(&date);

    auto hash = std::hash<std::string>{}(hashString);
    std::string filename = "Replays/" +  std::to_string(hash) + ".ctr";
    std::ofstream scorefile(filename, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
    scorefile.write(reinterpret_cast<const char*>(&value), sizeof value);
    scorefile.write(reinterpret_cast<const char*>(&date), sizeof time_t);
    const size_t seedLenght = seed.length();
    scorefile.write(reinterpret_cast<const char*>(&seedLenght), sizeof seedLenght);
    scorefile.write(seed.c_str(), seedLenght);
    scorefile.write(reinterpret_cast<const char*>(&fieldSize), sizeof fieldSize);
    scorefile.write(reinterpret_cast<const char*>(&roundtime), sizeof roundtime);
    scorefile.write(reinterpret_cast<const char*>(&colors), sizeof colors);
    scorefile.write(reinterpret_cast<const char*>(&misspenalty), sizeof misspenalty);
    scorefile.write(reinterpret_cast<const char*>(&misscount), sizeof misscount);
    const size_t size = replayData.size();
    scorefile.write(reinterpret_cast<const char*>(&size), sizeof size);
    scorefile.write(reinterpret_cast<const char*>(replayData.data()), sizeof(ReplayTick) * size);
    scorefile.close();
}

static void LoadScoreHeader(Score& score, std::ifstream& scorefile) {
    scorefile.read(reinterpret_cast<char*>(&score.value), sizeof score.value);
    scorefile.read(reinterpret_cast<char*>(&score.date), sizeof time_t);
    size_t seedLenght{};
    scorefile.read(reinterpret_cast<char*>(&seedLenght), sizeof seedLenght);
    score.seed.resize(seedLenght);
    scorefile.read(score.seed.data(), seedLenght);
    scorefile.read(reinterpret_cast<char*>(&score.fieldSize), sizeof score.fieldSize);
    scorefile.read(reinterpret_cast<char*>(&score.roundtime), sizeof score.roundtime);
    scorefile.read(reinterpret_cast<char*>(&score.colors), sizeof score.colors);
    scorefile.read(reinterpret_cast<char*>(&score.misspenalty), sizeof score.misspenalty);
    scorefile.read(reinterpret_cast<char*>(&score.misscount), sizeof score.misscount);
}

static void LoadScoreReplay(Score& score, std::ifstream& scorefile, int offset = 0) {
    size_t size{};
    scorefile.seekg(offset, std::ifstream::beg);
    scorefile.read(reinterpret_cast<char*>(&size), sizeof size);
    score.replayData.resize(size);
    scorefile.read(reinterpret_cast<char*>(score.replayData.data()), sizeof(ReplayTick) * size);
}

static std::ifstream OpenFileWithExpetions(const std::string& path) {
    std::ifstream scorefile(path, std::ifstream::binary | std::ifstream::in);      
    scorefile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    if (!scorefile.is_open()) {
        throw std::runtime_error(std::format("Could not open score {}", path));
    }
    return scorefile;
}

Score LoadScore(const std::string& path) {
    std::ifstream scorefile = OpenFileWithExpetions(path);
    Score score;
    LoadScoreHeader(score, scorefile);
    LoadScoreReplay(score, scorefile);
    scorefile.close();
    return score;
}

Score LoadScoreNoReplay(const std::string& path) {
    std::ifstream scorefile = OpenFileWithExpetions(path);
    Score score;
    LoadScoreHeader(score, scorefile);
    score.replayFileOffset = scorefile.tellg(); 
    scorefile.close();
    return score;
}

void LoadScoreReplay(Score& score, const std::string& path) {
    std::ifstream scorefile = OpenFileWithExpetions(path);
    if (!score.replayFileOffset) {
        throw std::runtime_error("Score does not have replay.");
    }
    LoadScoreReplay(score, scorefile, score.replayFileOffset.value());
    scorefile.close();
}