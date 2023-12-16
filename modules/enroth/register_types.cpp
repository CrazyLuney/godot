#include "register_types.h"

#include "height_map_ex_shape_3d.h"
#include "enroth_model.h"

namespace {

Ref<EnrothModelSaver> enroth_model_saver;
Ref<EnrothModelLoader> enroth_model_loader;

}

void initialize_enroth_module(ModuleInitializationLevel p_level) {
	
	switch (p_level) {
		case MODULE_INITIALIZATION_LEVEL_SERVERS:
			GDREGISTER_CLASS(EnrothModel)

			enroth_model_saver.instantiate();
			enroth_model_loader.instantiate();

			ResourceSaver::add_resource_format_saver(enroth_model_saver);
			ResourceLoader::add_resource_format_loader(enroth_model_loader);

			break;
		case MODULE_INITIALIZATION_LEVEL_SCENE:
			GDREGISTER_CLASS(HeightMapExShape3D)

			break;
		default:
			break;
	}
}

void uninitialize_enroth_module(ModuleInitializationLevel p_level) {
	switch (p_level) {
		case MODULE_INITIALIZATION_LEVEL_SERVERS:
			ResourceSaver::remove_resource_format_saver(enroth_model_saver);
			ResourceLoader::remove_resource_format_loader(enroth_model_loader);

			enroth_model_saver.unref();
			enroth_model_loader.unref();

			break;
		default:
			break;
	}
}
