#include "register_types.h"

#include "height_map_ex_shape_3d.h"

void initialize_enroth_module(ModuleInitializationLevel p_level) {
	switch (p_level) {
		case MODULE_INITIALIZATION_LEVEL_SCENE:
			GDREGISTER_CLASS(HeightMapExShape3D);
			break;
		default:
			break;
	}
}

void uninitialize_enroth_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}
