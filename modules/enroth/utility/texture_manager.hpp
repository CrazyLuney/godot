#pragma once

#include "sprite_importer.hpp"
#include "texture_importer.hpp"

#include "AssetTools/AssetLoader.hpp"

#include "core/templates/hash_map.h"
#include "scene/resources/image_texture.h"
#include "scene/resources/material.h"

class EnrothTextureManager {
	static constexpr uint16_t InvalidPaletteId = -1;
public:
	explicit EnrothTextureManager(enroth::asset_tools::AssetLoader &asset_loader);

	Ref<Texture2D> GetBitmapTexture(const String &texture_name) const;

	Ref<Image> GetOrCreateBitmapImage(const String &texture_name);
	Ref<Texture2D> GetOrCreateBitmapTexture(const String &texture_name);
	Ref<Material> GetOrCreateBitmapMaterial(const String &texture_name);

	Ref<Image> GetOrCreateSpriteBitmap(const String &sprite_name);
	Ref<Image> GetOrCreateSpriteImage(const String &sprite_name, uint16_t palette_id = InvalidPaletteId);
	Ref<Texture2D> GetOrCreateSpriteTexture(const String &sprite_name, uint16_t palette_id = InvalidPaletteId);

private:
	class Key {
	public:
		Key(enroth::io::LodEntryId entry_id) :
				_entry_id(entry_id) {
		}

		Key(enroth::io::LodEntryId entry_id, uint16_t palette_id) :
				_entry_id(entry_id),
				_palette_id(palette_id) {
		}

		uint32_t hash() const {
			uint32_t h;
			if constexpr (sizeof(_entry_id) == sizeof(uint64_t))
				h = hash_murmur3_one_64(_entry_id);
			else
				h = hash_murmur3_one_32(_entry_id);
			h = hash_murmur3_one_32(_palette_id, h);
			return hash_fmix32(h);
		}

		friend bool operator==(const Key &lhs, const Key &rhs) {
			return lhs._entry_id == rhs._entry_id && lhs._palette_id == rhs._palette_id;
		}

		friend bool operator!=(const Key &lhs, const Key &rhs) {
			return !(lhs == rhs);
		}

		friend bool operator<(const Key &lhs, const Key &rhs) {
			if (lhs._entry_id < rhs._entry_id)
				return true;
			if (rhs._entry_id < lhs._entry_id)
				return false;
			return lhs._palette_id < rhs._palette_id;
		}

		friend bool operator<=(const Key &lhs, const Key &rhs) {
			return !(rhs < lhs);
		}

		friend bool operator>(const Key &lhs, const Key &rhs) {
			return rhs < lhs;
		}

		friend bool operator>=(const Key &lhs, const Key &rhs) {
			return !(lhs < rhs);
		}

	private:
		enroth::io::LodEntryId _entry_id;
		uint16_t _palette_id = 0;
	};

	class KeyHasher {
	public:
		static _FORCE_INLINE_ uint32_t hash(const Key &key) { return key.hash(); }
	};

	struct SpriteHeader {
		SpriteHeader() = default;

		SpriteHeader(const enroth::io::LodSpriteFile &sprite) :
				width(static_cast<int>(sprite.width)),
				height(static_cast<int>(sprite.height)),
				palette_id(static_cast<uint16_t>(sprite.paletteId)) {
		}

		int width = 0;
		int height = 0;
		uint16_t palette_id = 0;
	};

	static String TranslateBitmapTextureName(const String &texture_name);

	static bool CheckSpriteImage(const SpriteHeader &header, const Ref<Image> &image);

	enroth::asset_tools::AssetLoader &_asset_loader;

	HashMap<Key, Ref<Image>, KeyHasher> _images;
	HashMap<Key, Ref<Image>, KeyHasher> _sprite_bitmaps;
	HashMap<Key, Ref<Image>, KeyHasher> _sprite_default_images;

	HashMap<Key, Ref<Texture2D>, KeyHasher> _textures;
	HashMap<Key, Ref<Texture2D>, KeyHasher> _sprite_default_textures;

	HashMap<Key, Ref<Material>, KeyHasher> _materials;

	HashMap<enroth::io::LodEntryId, SpriteHeader> _sprite_headers;
};
