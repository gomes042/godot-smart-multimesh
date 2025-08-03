#include "./register_types.h"

#include "godot_cpp/core/class_db.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

#include "resources/smart_multimesh_container_3d.h"
#include "smart_multimesh_instance_3d.h"

void smart_multimesh_initialize_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	// Resources
	GDREGISTER_CLASS(SmartMultiMeshContainer3D)

	GDREGISTER_CLASS(SmartMultiMeshInstance3D)

	//godot::UtilityFunctions::print("SmartMultiMesh: Library initialized.");
}

void smart_multimesh_uninitialize_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	//godot::UtilityFunctions::print("SmartMultiMesh: Library uninitialized.");
}

// Initialization.
extern "C" {
GDExtensionBool GDE_EXPORT smart_multimesh_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(smart_multimesh_initialize_module);
	init_obj.register_terminator(smart_multimesh_uninitialize_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}
