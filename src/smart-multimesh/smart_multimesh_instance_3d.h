#pragma once

#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/mesh.hpp"
#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/object.hpp"
#include "godot_cpp/classes/random_number_generator.hpp"
#include "godot_cpp/classes/rendering_server.hpp"
#include "godot_cpp/classes/world3d.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/transform3d.hpp"
#include "godot_cpp/variant/typed_array.hpp"

#include "resources/smart_multimesh_container_3d.h"

#include <vector>

using namespace godot;

class SmartMultiMeshInstance3D : public Node3D {
	GDCLASS(SmartMultiMeshInstance3D, Node3D);

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("set_containers", "containers"), &SmartMultiMeshInstance3D::set_containers);
		ClassDB::bind_method(D_METHOD("get_containers"), &SmartMultiMeshInstance3D::get_containers);

		ClassDB::bind_method(D_METHOD("get_internal_total_multimeshes_count"), &SmartMultiMeshInstance3D::get_internal_total_multimeshes_count);
		ClassDB::bind_method(D_METHOD("get_total_meshes_count"), &SmartMultiMeshInstance3D::get_total_meshes_count);

		ClassDB::bind_method(D_METHOD("get_container_index"), &SmartMultiMeshInstance3D::get_container_index);

		ADD_PROPERTY(
				PropertyInfo(Variant::STRING, "total_meshes", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY),
				"", "get_total_meshes_count");
		ADD_PROPERTY(
				PropertyInfo(Variant::STRING, "internal_multimeshes", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY),
				"", "get_internal_total_multimeshes_count");

		ADD_PROPERTY(
				PropertyInfo(Variant::ARRAY, "containers", PROPERTY_HINT_ARRAY_TYPE, "SmartMultiMeshContainer3D"),
				"set_containers",
				"get_containers");
	}

private:
	TypedArray<SmartMultiMeshContainer3D> containers;

	std::vector<RID> multimeshes;
	std::vector<RID> instances;

public:
	void set_containers(const TypedArray<SmartMultiMeshContainer3D> &p_containers) {
		// Disconnect from old containers
		/* 	for (int i = 0; i < containers.size(); i++) {
				Ref<SmartMultiMeshContainer3D> container = containers[i];
				if (container.is_valid()) {
					Object *obj = Object::cast_to<Object>(*container);
					if (obj && obj->is_connected("property_changed", callable_mp(this, &SmartMultiMeshInstance3D::recreate_multimeshes_from_containers))) {
						obj->disconnect("property_changed", callable_mp(this, &SmartMultiMeshInstance3D::recreate_multimeshes_from_containers));
					}
				}
			} */

		containers = p_containers;
		for (int i = 0; i < containers.size(); i++) {
			Ref<SmartMultiMeshContainer3D> container = containers[i];
			if (container.is_valid()) {
				container->set_instance(this);
			}
		}

		// Connect to new containers
		/* for (int i = 0; i < containers.size(); i++) {
			Ref<SmartMultiMeshContainer3D> container = containers[i];
			if (container.is_valid()) {
				Object *obj = Object::cast_to<Object>(*container);
				if (obj && !obj->is_connected("property_changed", callable_mp(this, &SmartMultiMeshInstance3D::recreate_multimeshes_from_containers))) {
					obj->connect("property_changed", callable_mp(this, &SmartMultiMeshInstance3D::recreate_multimeshes_from_containers));
				}
			}
		} */

		// Optional: Delay execution in editor mode
		// if (!Engine::get_singleton()->is_editor_hint()) {
		// 	recreate_multimeshes_from_containers();
		// } else {
		// 	call_deferred("recreate_multimeshes_from_containers"); // Delay until safe
		// }
	}

	TypedArray<SmartMultiMeshContainer3D> get_containers() const {
		return containers;
	}

	int get_internal_total_multimeshes_count() const {
		return static_cast<int>(multimeshes.size());
	}

	// TODO: Implement
	int get_total_meshes_count() {
		return 0;
	}

	void recreate_multimeshes_from_containers() {
		if (!is_inside_tree()) {
			// Prevent crashes if node isn't fully in scene tree yet
			return;
		}

		RenderingServer *rs = RenderingServer::get_singleton();

		for (const RID &instance : instances) {
			rs->free_rid(instance);
		}
		instances.clear();

		for (const RID &mm : multimeshes) {
			rs->free_rid(mm);
		}
		multimeshes.clear();

		if (containers.size() == 0) {
			//ERR_PRINT("No containers");
			return;
		}

		RID scenario_rid = get_world_3d()->get_scenario();

		Ref<RandomNumberGenerator> rng;
		rng.instantiate();
		rng->randomize();

		for (int i = 0; i < containers.size(); i++) {
			SmartMultiMeshContainer3D *container = Object::cast_to<SmartMultiMeshContainer3D>(containers[i]);

			Ref<Mesh> raw_mesh = container->get_mesh();
			if (raw_mesh == nullptr) {
				ERR_PRINT("Invalid mesh at index " + String::num(i));
				continue;
			}

			RID mm_rid = rs->multimesh_create();
			rs->multimesh_set_mesh(mm_rid, raw_mesh->get_rid());
			rs->multimesh_allocate_data(mm_rid, container->get_instance_count(), RenderingServer::MULTIMESH_TRANSFORM_3D);
			rs->multimesh_set_visible_instances(mm_rid, container->get_instance_count());

			for (int j = 0; j < container->get_instance_count(); j++) {
				Vector3 pos(
						rng->randf_range(-10.0, 10.0),
						rng->randf_range(-10.0, 10.0),
						rng->randf_range(-10.0, 10.0));
				Transform3D xform;
				xform.origin = pos;
				//rs->multimesh_instance_set_transform(mm_rid, j, xform);
			}

			RID instance_rid = rs->instance_create();
			rs->instance_set_base(instance_rid, mm_rid);
			rs->instance_set_scenario(instance_rid, scenario_rid);
			rs->instance_set_transform(instance_rid, get_global_transform());
			rs->instance_set_visible(instance_rid, true);
			rs->instance_geometry_set_cast_shadows_setting(instance_rid, RenderingServer::SHADOW_CASTING_SETTING_ON);

			multimeshes.push_back(mm_rid);
			instances.push_back(instance_rid);
		}
	}

	void _ready() override {
		/* if (Engine::get_singleton()->is_editor_hint()) {
			return;
		} */

		this->notify_property_list_changed();

		recreate_multimeshes_from_containers();

		// call gdscript init
		if (has_method("init")) {
			call("init");
		}
	}

	void _exit_tree() override {
		RenderingServer *rs = RenderingServer::get_singleton();

		for (const RID &instance : instances) {
			rs->free_rid(instance);
		}
		instances.clear();

		for (const RID &mm : multimeshes) {
			rs->free_rid(mm);
		}
		multimeshes.clear();
	}

	int get_container_index(SmartMultiMeshContainer3D *container) const { return static_cast<int>(containers.find(container)); }

	void set_instance_transform_by_container_and_instance_index(int container_index, int instance_index, const Transform3D &transform) {
		RID multimesh_index = multimeshes[container_index];
		RenderingServer *rs = RenderingServer::get_singleton();
		rs->multimesh_instance_set_transform(multimesh_index, instance_index, transform);
	}

	//void set_instance_transform(SmartMultiMeshContainer3D *container, int instance_index, const Transform3D &transform);
};
