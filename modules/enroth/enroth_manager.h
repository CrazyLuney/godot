#pragma once

#include "core/object/class_db.h"
#include "core/object/object.h"

#include "AssetTools/AssetLoader.hpp"

class EnrothManager : public Object {
	GDCLASS(EnrothManager, Object)
private:
	static EnrothManager *singleton;

	enroth::asset_tools::AssetLoader asset_loader;

	enroth::data::mm7::SpriteFrameTable sft;
	enroth::data::mm7::TileDescTable tdt;
	enroth::data::mm7::DecorationDescTable ddt;

	void _initialize();

public:
	_FORCE_INLINE_ static EnrothManager *get_singleton() { return singleton; }

	EnrothManager();
	~EnrothManager() override;

	bool is_valid() const {
		return asset_loader.IsValid();
	}

	const auto &get_asset_loader() const { return asset_loader; }
	const auto &get_sprite_frame_table() const { return sft; }
	const auto &get_tile_desc_table() const { return tdt; }
	const auto &get_decoration_desc_table() const { return ddt; }
};
