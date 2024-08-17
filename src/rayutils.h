#pragma once
#include "raylib.h"

namespace Rayutils {
	inline int GetDisplayWidth() {
		if (IsWindowFullscreen()) {
			return GetMonitorWidth(GetCurrentMonitor());
		}
		return GetScreenWidth();
	}

	inline int GetDisplayHeight() {
		if (IsWindowFullscreen()) {
			return GetMonitorHeight(GetCurrentMonitor());
		}
		return GetScreenHeight();
	}

	inline void DrawOutlinedText(const char* text, int posX, int posY, int fontSize, Color color, int outlineSize, Color outlineColor) {
		DrawText(text, posX - outlineSize, posY - outlineSize, fontSize, outlineColor);
		DrawText(text, posX + outlineSize, posY - outlineSize, fontSize, outlineColor);
		DrawText(text, posX - outlineSize, posY + outlineSize, fontSize, outlineColor);
		DrawText(text, posX + outlineSize, posY + outlineSize, fontSize, outlineColor);
		DrawText(text, posX, posY, fontSize, color);
	}

	inline void DrawFullTexture(const Texture2D& texture, const Rectangle& position, Vector2 origin = { 0, 0 }, float rotation = 0.f, Color color = WHITE) {
		DrawTexturePro(
			texture,
			{ .x = 0, .y = 0, .width = (float)texture.width, .height = (float)texture.height },
			position, origin, rotation, color
		);
	}

	struct Vector2i {
		int x{};
		int y{};

		static Vector2i CompareUnit(Vector2i op1, Vector2i op2) {
			Vector2i result;

			if (op2.x - op1.x < 0) {
				result.x = -1;
			}
			else if (op2.x - op1.x > 0) {
				result.x = 1;
			}

			if (op2.y - op1.y < 0) {
				result.y = -1;
			}
			else if (op2.y - op1.y > 0) {
				result.y = 1;
			}

			return result;
		}

		Vector2i operator+(const Vector2i& other) const {
			return { x + other.x, y + other.y };
		}

		bool operator==(const Vector2i& other) const {
			return (x == other.x) && (y == other.y);
		}
	};
}