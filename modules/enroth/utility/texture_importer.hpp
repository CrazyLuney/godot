#pragma once

#include "AssetTools/AssetLoader.hpp"

#include "core/io/image.h"

#include <unordered_set>

class EnrothTextureImporter {
public:
	Ref<Image> Import(const enroth::io::LodBitmapFile &bitmap) {
		assert(!!bitmap.valid);
		assert(!!bitmap.pixels);
		assert(!!bitmap.palette);

		assert(bitmap.flags & 0x0010);

		const auto w = static_cast<std::size_t>(bitmap.width);
		const auto h = static_cast<std::size_t>(bitmap.height);

		PackedByteArray pixels;

		pixels.resize(w * h * 4);

		AssignBitmap(bitmap);

		if (IsTransparentTexture(bitmap)) {
			BuildTransparentTextureData(pixels.ptrw());

			print_line("Transparent texture: ", bitmap.name.c_str());
		} else {
			BuildTextureData(pixels.ptrw());

			print_line("Solid texture: ", bitmap.name.c_str());
		}

		return Image::create_from_data(w, h, false, Image::FORMAT_RGBA8, pixels);
	}

private:
	static bool IsTransparentTexture(const enroth::io::LodBitmapFile &bitmap) {
		static const std::unordered_set<std::string, enroth::utility::string_hash_lc, enroth::utility::string_equal_to_lc> transparent_textures{
			"hwtrdre",
			"hwtrdrne",
			"hwtrdrs",
			"hwtrdrsw",
			"hwtrdrxne",
			"hwtrdrxse",
			"hwtrdrn",
			"hwtrdrnw",
			"hwtrdrse",
			"hwtrdrw",
			"hwtrdrxnw",
			"hwtrdrxsw"
		};

		if (bitmap.flags & 0x0200)
			return true;

		return transparent_textures.contains(bitmap.name);
	}

	void AssignBitmap(const enroth::io::LodBitmapFile &bitmap) {
		_indices = static_cast<const std::uint8_t *>(bitmap.pixels.data());
		_palette = static_cast<const enroth::asset_tools::Palette *>(bitmap.palette.data());
		_width = bitmap.width;
		_height = bitmap.height;
	}

	void BuildTextureData(std::uint8_t *const pixels) const {
		const auto p = _palette->colors;

		for (unsigned y = 0; y < _height; ++y) {
			auto i = _indices + y * _width;
			auto o = pixels + y * _width * 4;

			for (unsigned x = 0; x < _width; ++x, i += 1, o += 4) {
				const auto &c = p[*i];

				o[0] = c.r;
				o[1] = c.g;
				o[2] = c.b;
				o[3] = 255;
			}
		}
	}

	void BuildTransparentTextureData(std::uint8_t *const pixels) const {
		const auto p = _palette->colors;

		for (unsigned y = 0; y < _height; ++y) {
			auto i = _indices + y * _width;
			auto o = pixels + y * _width * 4;

			for (unsigned x = 0; x < _width; ++x, i += 1, o += 4) {
				if (*i == 0) {
					const auto c{ CalculateTransparentPixelColor(x, y) };

					o[0] = c.r;
					o[1] = c.g;
					o[2] = c.b;
					o[3] = 0;
				} else {
					const auto &c = p[*i];

					o[0] = c.r;
					o[1] = c.g;
					o[2] = c.b;
					o[3] = 255;
				}
			}
		}
	}

	enroth::asset_tools::RGB CalculateTransparentPixelColor(const unsigned x, const unsigned y) const {
		constexpr std::array<int, 8> dx{ -1, 0, 1, -1, 1, -1, 0, 1 };
		constexpr std::array<int, 8> dy{ -1, -1, -1, 0, 0, 1, 1, 1 };

		unsigned r = 0;
		unsigned g = 0;
		unsigned b = 0;
		unsigned c = 0;

		const int xx = static_cast<int>(x);
		const int yy = static_cast<int>(y);
		const int ww = static_cast<int>(_width);
		const int hh = static_cast<int>(_height);

		for (std::size_t i = 0; i < 8; ++i) {
			const int px = xx + dx[i];
			const int py = yy + dy[i];

			if (px < 0)
				continue;
			if (py < 0)
				continue;
			if (px >= ww)
				continue;
			if (py >= hh)
				continue;

			const auto &pi = _indices[py * ww + px];

			if (pi == 0)
				continue;

			const auto &pc = _palette->colors[pi];

			r += pc.r;
			g += pc.g;
			b += pc.b;
			c += 1;
		}

		if (c > 0) {
			r /= c;
			g /= c;
			b /= c;
		}

		return { static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g), static_cast<std::uint8_t>(b) };
	}

	const std::uint8_t *_indices = nullptr;
	const enroth::asset_tools::Palette *_palette = nullptr;
	unsigned _width = 0;
	unsigned _height = 0;
};
