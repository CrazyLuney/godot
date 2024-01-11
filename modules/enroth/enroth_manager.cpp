#include "enroth_manager.h"

#include "core/config/project_settings.h"

namespace {

std::filesystem::path _get_data_path() {
	const String path_setting{ GLOBAL_GET("enroth/general/data_path") };
	return { path_setting.ascii().ptr() };
}

} //namespace

EnrothManager *EnrothManager::singleton = nullptr;

EnrothManager::EnrothManager() :
		asset_loader{ _get_data_path() },
		texture_manager{ asset_loader } {
	CRASH_COND(singleton != nullptr);

	singleton = this;

	{
		const auto& data_path = asset_loader.GetDataDir();

		if (is_valid()) {
			print_line(vformat("enroth: data path is %s", data_path.generic_string().c_str()));
		} else {
			WARN_PRINT(vformat("enroth: invalid data path %s", data_path.generic_string().c_str()));
		}
	}
}

EnrothManager::~EnrothManager() {
	CRASH_COND(singleton == nullptr);

	singleton = nullptr;
}
