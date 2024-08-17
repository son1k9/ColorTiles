#pragma once
#include <exception>
#include <iostream>
#include <map>
#include <format>
#include <string>
#include "raylib.h"

namespace Settings {
	class Skin {
	private:
		Sound hitSound{};
		Sound missSound{};

		Texture2D background{};
		Texture2D colorTiles[10]{};
		Texture2D colorTilesRemoving[10]{};

		Texture2D emptyTiles[2]{};
		Texture2D trail[2]{};

		Texture2D cursor;
	public:
		const std::string name;

		Skin(const std::string& skinName);
		Skin(const Skin& other) = delete;
		Skin(Skin&& other) = delete;
		Skin& operator=(const Skin& other) = delete;
		Skin& operator=(Skin&& other) = delete;
		~Skin();

		const Sound& HitSound() const {
			return hitSound;
		}

		const Sound& MissSound() const {
			return missSound;
		}

		const Texture2D& Background() const {
			return background;
		}

		const Texture2D& ColorTile(int i) const {
			if (i < 0 || i > 9) {
				throw std::out_of_range(std::format("i must be >= 0 and <= {}, i={}", 9, i));
			}
			return colorTiles[i];
		}

		const Texture2D& ColorTileRemoving(int i) const {
			if (i < 0 || i > 9) {
				throw std::out_of_range(std::format("i must be >= 0 and <= {}, i={}", 9, i));
			}
			return colorTilesRemoving[i];
		}

		const Texture2D& EmptyTile(int i) const {
			if (i < 0 || i > 1) {
				throw std::out_of_range(std::format("i must be >= 0 and <= {}, i={}", 1, i));
			}
			return emptyTiles[i];
		}

		const Texture2D& Trail(int i) const {
			if (i < 0 || i > 1) {
				throw std::out_of_range(std::format("i must be >= 0 and <= {}, i={}", 1, i));
			}
			return trail[i];
		}

		const Texture2D& Cursor() const {
			return cursor;
		}
	};

	std::unique_ptr<Skin> TryLoadSkinOrDefaut(const std::string& skinName);
	//Throws std::runtime_error if could not open given folder
	std::unique_ptr<Skin> TryLoadSkin(const std::string& skinName);

	extern std::unique_ptr<Skin> skin;
	extern float cursorSize;
	extern int fieldSize;
	extern int roundTime;
	extern int colors;
	extern bool useCustomCursor;

	std::map<std::string, std::string> ReadCfg(const std::string& path);
	void WriteCfg(std::map<std::string, std::string> settings, const std::string& path);
	void ReadSettingsFromMap(std::map<std::string, std::string>& values);
	std::map<std::string, std::string> SettingsToMap();
}