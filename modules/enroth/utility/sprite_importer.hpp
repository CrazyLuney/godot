#pragma once

#include "AssetTools/AssetLoader.hpp"

#include "core/io/image.h"

class EnrothSpriteImporter {
public:
	static Ref<Image> ImportBitmap(const enroth::io::LodSpriteFile &sprite) {
		assert(!!sprite.valid);
		assert(!!sprite.lines);

		const auto w = static_cast<int>(sprite.width);
		const auto h = static_cast<int>(sprite.height);

		PackedByteArray indices;

		indices.resize(w * h);
		indices.fill(0);

		{
			const auto lines = static_cast<const enroth::data::LodSpriteLine *>(sprite.lines.data());
			const auto data = static_cast<const std::uint8_t *>(sprite.data.data());

			for (unsigned y = 0; y < sprite.height; ++y) {
				const auto &line = lines[y];

				if (line.End < 0 || line.Begin < 0)
					continue;
				if (line.End < line.Begin)
					continue;

				const auto i = data + line.Offset;
				const auto o = indices.ptrw() + y * w + line.Begin;

				std::memcpy(o, i, line.End - line.Begin + 1);
			}
		}

		return Image::create_from_data(w, h, false, Image::FORMAT_R8, indices);
	}

	EnrothSpriteImporter(enroth::asset_tools::AssetLoader &asset_loader) :
			_asset_loader{ asset_loader } {
	}

	Ref<Image> Import(const enroth::io::LodSpriteFile &sprite) {
		return Import(sprite, sprite.paletteId);
	}

	Ref<Image> Import(const enroth::io::LodSpriteFile &sprite, const std::uint16_t palette_id) {
		assert(!!sprite.valid);
		assert(!!sprite.lines);

		const auto w = static_cast<int>(sprite.width);
		const auto h = static_cast<int>(sprite.height);

		PackedByteArray indices;

		indices.resize(w * h);
		indices.fill(0);

		{
			const auto lines = static_cast<const enroth::data::LodSpriteLine *>(sprite.lines.data());
			const auto data = static_cast<const std::uint8_t *>(sprite.data.data());

			for (int y = 0; y < h; ++y) {
				const auto &line = lines[y];

				if (line.End < 0 || line.Begin < 0)
					continue;
				if (line.End < line.Begin)
					continue;

				const auto i = data + line.Offset;
				const auto o = indices.ptrw() + y * w + line.Begin;

				std::memcpy(o, i, line.End - line.Begin + 1);
			}
		}

		PackedByteArray pixels;

		pixels.resize(w * h * 4);

		_indices = indices.ptr();
		_palette = _asset_loader.GetPalette(palette_id == 0 ? sprite.paletteId : palette_id);
		_width = sprite.width;
		_height = sprite.height;

		if (!_palette)
			_palette = _asset_loader.GetPalette(0);

		BuildTransparentTextureData(pixels.ptrw());

		return Image::create_from_data(w, h, false, Image::FORMAT_RGBA8, pixels);
	}

	Ref<Image> Import(const Ref<Image> &bitmap, const std::uint16_t palette_id) {
		if (bitmap.is_null())
			return {};

		const auto w = bitmap->get_width();
		const auto h = bitmap->get_height();

		const auto bd{ bitmap->get_data() };

		PackedByteArray pixels;

		pixels.resize(w * h * 4);

		_indices = bd.ptr();
		_palette = _asset_loader.GetPalette(palette_id);
		_width = w;
		_height = h;

		if (!_palette)
			_palette = _asset_loader.GetPalette(0);

		BuildTransparentTextureData(pixels.ptrw());

		return Image::create_from_data(w, h, false, Image::FORMAT_RGBA8, pixels);
	}

	Ref<Image> Import(const enroth::io::LodSpriteFile &sprite, const Ref<Image> &bitmap, const std::uint16_t palette_id) {
		if (bitmap.is_null())
			return Import(sprite, palette_id);

		const auto w = static_cast<int>(sprite.width);
		const auto h = static_cast<int>(sprite.height);

		if (bitmap->get_width() != w || bitmap->get_height() != h)
			return {};

		const auto bd{ bitmap->get_data() };

		PackedByteArray pixels;

		pixels.resize(w * h * 4);

		_indices = bd.ptr();
		_palette = _asset_loader.GetPalette(palette_id == 0 ? sprite.paletteId : palette_id);
		_width = sprite.width;
		_height = sprite.height;

		if (!_palette)
			_palette = _asset_loader.GetPalette(0);

		BuildTransparentTextureData(pixels.ptrw());

		return Image::create_from_data(w, h, false, Image::FORMAT_RGBA8, pixels);
	}

private:
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

	enroth::asset_tools::AssetLoader &_asset_loader;

	const std::uint8_t *_indices = nullptr;
	const enroth::asset_tools::Palette *_palette = nullptr;
	unsigned _width = 0;
	unsigned _height = 0;
};
