#include "enroth_manager.h"

#include "core/config/project_settings.h"

namespace {

std::filesystem::path _get_data_path() {
	const String path_setting{ GLOBAL_GET("enroth/general/data_path") };
	const std::filesystem::path path{ path_setting.ascii().ptr() };
	return path;
}

} //namespace

EnrothManager *EnrothManager::singleton = nullptr;

void EnrothManager::_initialize() {
	const auto data_path{ _get_data_path() };

	if (is_valid()) {
		print_line(vformat("enroth: data path is %s", data_path.generic_string().c_str()));

		sft = asset_loader.LoadSpriteFrameTable();
		tdt = asset_loader.LoadTileDescTable();
		ddt = asset_loader.LoadDecorationDescTable();
	} else {
		WARN_PRINT(vformat("enroth: invalid data path %s", data_path.generic_string().c_str()));
	}
}

EnrothManager::EnrothManager() :
		asset_loader{ _get_data_path() } {
	singleton = this;

	_initialize();
}

EnrothManager::~EnrothManager() {
	singleton = nullptr;
}
