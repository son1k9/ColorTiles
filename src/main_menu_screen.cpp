#include "raygui.h"
#include "settings.h"
#include "screen.h"
#include "rayutils.h"

extern bool close;

void MainMenuScreen::Quit() {
	Settings::WriteCfg(Settings::SettingsToMap(), "settings.cfg");
	close = true;
}

MainMenuScreen::MainMenuScreen() {
}

void MainMenuScreen::Update() {
	if (showDialogWindow) {
		if (IsKeyPressed(KEY_ESCAPE)) {
			showDialogWindow = false;
		}
		if (IsKeyPressed(KEY_ENTER) && !fieldSizeEdit && !colorsEdit && !roundTimeEdit) {
			RequestScreenChange(Screens::Gameplay);
		}
	}
	else {
		if (IsKeyPressed(KEY_ESCAPE)) {
			Quit();
		}
		if (IsKeyPressed(KEY_ENTER)) {
			showDialogWindow = true;
		}
	}
}

void MainMenuScreen::Draw() {
	GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(BLACK));
	GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, ColorToInt(BLACK));
	GuiSetStyle(DEFAULT, TEXT_SIZE, 40);
	GuiSetStyle(DEFAULT, BORDER_WIDTH, 2);

	int width = Rayutils::GetDisplayWidth();
	int height = Rayutils::GetDisplayHeight();

	ClearBackground(RAYWHITE);

	int buttomWidth = 300;
	int buttonHeight = 80;
	int gap = 25;

	if (GuiButton(
		{ .x = (width - buttomWidth) / 2.f, .y = (height - buttonHeight) / 2.f, .width = (float)buttomWidth, .height = (float)buttonHeight },
		"Play") && !showDialogWindow) {
		showDialogWindow = true;
	}

	if (GuiButton(
		{ .x = (width - buttomWidth) / 2.f, .y = (height - buttonHeight) / 2.f + buttonHeight + gap, .width = (float)buttomWidth, .height = (float)buttonHeight },
		"Scores") && !showDialogWindow) {
		RequestScreenChange(Screens::Scores);
	}

	if (GuiButton({ .x = (width - buttomWidth) / 2.f, .y = (height - buttonHeight) / 2.f + 2 * (buttonHeight + gap), .width = (float)buttomWidth, .height = (float)buttonHeight },
		"Settings") && !showDialogWindow) {
		RequestScreenChange(Screens::Settings);
	}

	if (GuiButton({ .x = (width - buttomWidth) / 2.f, .y = (height - buttonHeight) / 2.f + 3 * (buttonHeight + gap), .width = (float)buttomWidth, .height = (float)buttonHeight },
		"Quit") && !showDialogWindow) {
		Quit();
	}

	if (showDialogWindow) {
		GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
		GuiSetStyle(DEFAULT, BORDER_WIDTH, 1);
		int windowWidth = 328;
		int windowHeight = 204;
		Vector2 dialogLeftTopCorner{ (width - windowWidth) / 2, (height - windowHeight) / 2 };
		showDialogWindow = !GuiWindowBox(
			{ .x = dialogLeftTopCorner.x, .y = dialogLeftTopCorner.y, .width = (float)windowWidth, .height = (float)windowHeight },
			"Gameplay settings");
		GuiLabel({ .x = dialogLeftTopCorner.x + 24, .y = dialogLeftTopCorner.y + 40, .width = 150, .height = 24 }, "Field size");
		if (GuiSpinner(
			{ .x = dialogLeftTopCorner.x + 184, .y = dialogLeftTopCorner.y + 40, .width = 120, .height = 24 },
			NULL, &Settings::fieldSize, 1, 20, fieldSizeEdit)) {
			fieldSizeEdit = !fieldSizeEdit;
		}

		GuiLabel({ .x = dialogLeftTopCorner.x + 24, .y = dialogLeftTopCorner.y + 80, .width = 150, .height = 24 }, "Colors");
		if (
			GuiSpinner({ .x = dialogLeftTopCorner.x + 184, .y = dialogLeftTopCorner.y + 80, .width = 120, .height = 24 },
				NULL, &Settings::colors, 1, 10, colorsEdit)) {
			colorsEdit = !colorsEdit;
		}

		GuiLabel({ .x = dialogLeftTopCorner.x + 24, .y = dialogLeftTopCorner.y + 120, .width = 150, .height = 24 }, "Round time");
		if (GuiSpinner(
			{ .x = dialogLeftTopCorner.x + 184, .y = dialogLeftTopCorner.y + 120, .width = 120, .height = 24 },
			NULL, &Settings::roundTime, 1, 120, roundTimeEdit)) {
			roundTimeEdit = !roundTimeEdit;
		}

		if (GuiButton(
			{ .x = dialogLeftTopCorner.x + (windowWidth - 120) / 2, .y = dialogLeftTopCorner.y + 160, .width = 120, .height = 24 },
			"Play")) {
			RequestScreenChange(Screens::Gameplay);
		}
	}
}