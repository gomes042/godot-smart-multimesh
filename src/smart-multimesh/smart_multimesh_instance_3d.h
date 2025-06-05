#pragma once

#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/mesh.hpp"
#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/object.hpp"
#include "godot_cpp/classes/random_number_generator.hpp"
#include "godot_cpp/classes/rendering_server.hpp"
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/classes/world3d.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/transform3d.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

#include <vector>

using namespace godot;

class SmartMultiMeshContainer3D : public Resource {
	GDCLASS(SmartMultiMeshContainer3D, Resource);

	Ref<Mesh> mesh;
	int instance_count = 0;

	bool use_indirect = false;
	godot::RenderingServer::MultimeshTransformFormat transform_format = RenderingServer::MULTIMESH_TRANSFORM_3D;
	bool use_colors = false;
	bool use_custom_data = false;

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &SmartMultiMeshContainer3D::set_mesh);
		ClassDB::bind_method(D_METHOD("get_mesh"), &SmartMultiMeshContainer3D::get_mesh);

		ClassDB::bind_method(D_METHOD("set_instance_count", "count"), &SmartMultiMeshContainer3D::set_instance_count);
		ClassDB::bind_method(D_METHOD("get_instance_count"), &SmartMultiMeshContainer3D::get_instance_count);

		ClassDB::bind_method(D_METHOD("set_use_indirect", "enabled"), &SmartMultiMeshContainer3D::set_use_indirect);
		ClassDB::bind_method(D_METHOD("get_use_indirect"), &SmartMultiMeshContainer3D::get_use_indirect);

		ClassDB::bind_method(D_METHOD("set_transform_format", "format"), &SmartMultiMeshContainer3D::set_transform_format);
		ClassDB::bind_method(D_METHOD("get_transform_format"), &SmartMultiMeshContainer3D::get_transform_format);

		ClassDB::bind_method(D_METHOD("set_use_colors", "enabled"), &SmartMultiMeshContainer3D::set_use_colors);
		ClassDB::bind_method(D_METHOD("get_use_colors"), &SmartMultiMeshContainer3D::get_use_colors);

		ClassDB::bind_method(D_METHOD("set_use_custom_data", "enabled"), &SmartMultiMeshContainer3D::set_use_custom_data);
		ClassDB::bind_method(D_METHOD("get_use_custom_data"), &SmartMultiMeshContainer3D::get_use_custom_data);

		ADD_PROPERTY(PropertyInfo(Variant::INT, "instance_count", PROPERTY_HINT_RANGE, "0,100000000,1"),
				"set_instance_count", "get_instance_count");
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");

		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_indirect"), "set_use_indirect", "get_use_indirect");

		ADD_PROPERTY(PropertyInfo(Variant::INT, "transform_format", PROPERTY_HINT_ENUM, "2D,3D", PROPERTY_USAGE_INTERNAL),
				"set_transform_format", "get_transform_format");

		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_colors"), "set_use_colors", "get_use_colors");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_custom_data"), "set_use_custom_data", "get_use_custom_data");

		ADD_SIGNAL(MethodInfo("property_changed"));
	}

public:
	void _get_property_list(List<PropertyInfo> *p_list) const {
		if (!use_indirect) {
			p_list->push_back(PropertyInfo(Variant::INT, "transform_format", PROPERTY_HINT_ENUM, "2D,3D"));
		}
	}

	// Mesh
	void set_mesh(const Ref<Mesh> &p_mesh) {
		mesh = p_mesh;
		emit_signal("property_changed");
	}
	Ref<Mesh> get_mesh() const { return mesh; }

	// Instance Count
	void set_instance_count(int count) {
		instance_count = count;
		emit_signal("property_changed");
	}
	int get_instance_count() const { return instance_count; }

	// Use Indirect
	void set_use_indirect(bool p_use_indirect) {
		use_indirect = p_use_indirect;
		emit_signal("property_changed");
		notify_property_list_changed();
	}
	bool get_use_indirect() const { return use_indirect; }

	// Transform Format
	void set_transform_format(godot::RenderingServer::MultimeshTransformFormat p_format) {
		transform_format = p_format;
		emit_signal("property_changed");
	}
	godot::RenderingServer::MultimeshTransformFormat get_transform_format() const { return transform_format; }

	// Use Colors
	void set_use_colors(bool p_use_colors) {
		use_colors = p_use_colors;
		emit_signal("property_changed");
	}
	bool get_use_colors() const { return use_colors; }

	// Use Custom Data
	void set_use_custom_data(bool p_use_custom_data) {
		use_custom_data = p_use_custom_data;
		emit_signal("property_changed");
	}
	bool get_use_custom_data() const { return use_custom_data; }
};

class SmartMultiMeshInstance3D : public Node3D {
	GDCLASS(SmartMultiMeshInstance3D, Node3D);

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("set_containers", "containers"), &SmartMultiMeshInstance3D::set_containers);
		ClassDB::bind_method(D_METHOD("get_containers"), &SmartMultiMeshInstance3D::get_containers);

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
		for (int i = 0; i < containers.size(); i++) {
			Ref<SmartMultiMeshContainer3D> container = containers[i];
			if (container.is_valid()) {
				Object *obj = Object::cast_to<Object>(*container);
				if (obj && obj->is_connected("property_changed", callable_mp(this, &SmartMultiMeshInstance3D::recreate_multimeshes_from_containers))) {
					obj->disconnect("property_changed", callable_mp(this, &SmartMultiMeshInstance3D::recreate_multimeshes_from_containers));
				}
			}
		}

		containers = p_containers;

		// Connect to new containers
		for (int i = 0; i < containers.size(); i++) {
			Ref<SmartMultiMeshContainer3D> container = containers[i];
			if (container.is_valid()) {
				Object *obj = Object::cast_to<Object>(*container);
				if (obj && !obj->is_connected("property_changed", callable_mp(this, &SmartMultiMeshInstance3D::recreate_multimeshes_from_containers))) {
					obj->connect("property_changed", callable_mp(this, &SmartMultiMeshInstance3D::recreate_multimeshes_from_containers));
				}
			}
		}

		// Optional: Delay execution in editor mode
		if (!Engine::get_singleton()->is_editor_hint()) {
			recreate_multimeshes_from_containers();
		} else {
			call_deferred("recreate_multimeshes_from_containers"); // Delay until safe
		}
	}

	TypedArray<SmartMultiMeshContainer3D> get_containers() const {
		return containers;
	}

	int get_interna_multimeshes_count() const {
		return static_cast<int>(multimeshes.size());
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
				rs->multimesh_instance_set_transform(mm_rid, j, xform);
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
};
