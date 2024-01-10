#include "register_types.h"
#include "core/config/project_settings.h"

#include "enroth_height_map_shape_3d.h"
#include "enroth_model.h"
#include "enroth_manager.h"

namespace {

EnrothManager* enroth_manager = nullptr;

Ref<EnrothModelSaver> enroth_model_saver;
Ref<EnrothModelLoader> enroth_model_loader;

}

void initialize_enroth_module_servers() {
	GLOBAL_DEF_RST(PropertyInfo(Variant::STRING, "enroth/general/data_path", PROPERTY_HINT_GLOBAL_DIR), "d:\\Games\\MMVII\\DAT\\");

	enroth_manager = memnew(EnrothManager);

	GDREGISTER_CLASS(EnrothModel)

	enroth_model_saver.instantiate();
	enroth_model_loader.instantiate();

	ResourceSaver::add_resource_format_saver(enroth_model_saver);
	ResourceLoader::add_resource_format_loader(enroth_model_loader);
}

void initialize_enroth_module_scene() {
	GLOBAL_DEF(PropertyInfo(Variant::INT, "enroth/general/point_unit_size", PROPERTY_HINT_RANGE, "16,2048,16,or_greater"), 512);
	GLOBAL_DEF(PropertyInfo(Variant::INT, "enroth/general/vector_unit_size", PROPERTY_HINT_RANGE, "16,65536,16,or_greater"), 65536);
	GLOBAL_DEF(PropertyInfo(Variant::INT, "enroth/height_map/cell_size", PROPERTY_HINT_RANGE, "16,2048,16,or_greater"), 512);
	GLOBAL_DEF(PropertyInfo(Variant::INT, "enroth/height_map/cell_height", PROPERTY_HINT_RANGE, "16,2048,16,or_greater"), 32);

	GDREGISTER_CLASS(EnrothHeightMapShape3D)
}

void uninitialize_enroth_module_servers() {
	ResourceSaver::remove_resource_format_saver(enroth_model_saver);
	ResourceLoader::remove_resource_format_loader(enroth_model_loader);

	enroth_model_saver.unref();
	enroth_model_loader.unref();

	memdelete(enroth_manager);

	enroth_manager = nullptr;
}

void initialize_enroth_module(ModuleInitializationLevel p_level) {
	
	switch (p_level) {
		case MODULE_INITIALIZATION_LEVEL_SERVERS:
			initialize_enroth_module_servers();
			break;
		case MODULE_INITIALIZATION_LEVEL_SCENE:
			initialize_enroth_module_scene();
			break;
		default:
			break;
	}
}

void uninitialize_enroth_module(ModuleInitializationLevel p_level) {
	switch (p_level) {
		case MODULE_INITIALIZATION_LEVEL_SERVERS:
			uninitialize_enroth_module_servers();
			break;
		default:
			break;
	}
}
