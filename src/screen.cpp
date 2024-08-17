#include "screen.h"

extern bool changeScreen;
extern Screens changeTo;

void RequestScreenChange(Screens screen) {
    changeScreen = true;
    changeTo = screen;
}

extern bool playReplay;
extern Score replay;
void RequestReplay(Score&& score) {
    playReplay = true;
    replay = std::move(score);
}