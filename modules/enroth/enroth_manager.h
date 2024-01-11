#pragma once

#include "core/object/class_db.h"
#include "core/object/object.h"

#include "utility/texture_manager.hpp"

class EnrothManager : public Object {
	GDCLASS(EnrothManager, Object)
private:
	static EnrothManager *singleton;

	enroth::asset_tools::AssetLoader asset_loader;
	EnrothTextureManager texture_manager;

public:
	_FORCE_INLINE_ static EnrothManager *get_singleton() { return singleton; }

	_FORCE_INLINE_ static const auto &get_sprite_frame_table() { return enroth::data::mm7::DataRoot::GetInstance()._SpriteFrameTable; }
	_FORCE_INLINE_ static const auto &get_tile_desc_table() { return enroth::data::mm7::DataRoot::GetInstance()._TileDescTable; }
	_FORCE_INLINE_ static const auto &get_decoration_desc_table() { return enroth::data::mm7::DataRoot::GetInstance()._DecorationDescTable; }

	EnrothManager();
	~EnrothManager() override;

	bool is_valid() const {
		return asset_loader.IsValid();
	}

	auto &get_asset_loader() { return asset_loader; }
	const auto &get_asset_loader() const { return asset_loader; }

	auto &get_texture_manager() { return texture_manager; }
	const auto &get_texture_manager() const { return texture_manager; }
};
