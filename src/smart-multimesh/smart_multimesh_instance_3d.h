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

#include "godot_cpp/variant/utility_functions.hpp"
#include "resources/smart_multimesh_container_3d.h"

#include <map>
#include <unordered_map>
#include <vector>

using namespace godot;

namespace std {
template <>
struct hash<RID> {
	std::size_t operator()(const RID &rid) const noexcept {
		return std::hash<uint64_t>()(rid.get_id());
	}
};
} //namespace std

#define MAX_MESHES_PER_MULTIMESH 1000000

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

	std::unordered_map<RID, int> multimeshes; // <multimesh RID, container index>
	std::vector<RID> rs_instances;

	std::unordered_map<RID, int> get_multimeshes_associated_with_container(int container_index) {
		std::unordered_map<RID, int> result;
		for (const auto &m : multimeshes) {
			if (m.second == container_index) {
				result[m.first] = m.second;
			}
		}
		return result;
	}

	bool has_multimesh_for_container_and_instance_index(int container_index, int instance_index) {
		auto unordered_associated_multimeshes = get_multimeshes_associated_with_container(container_index);
		if (unordered_associated_multimeshes.empty()) {
			return false;
		}

		int multimesh_index = instance_index / MAX_MESHES_PER_MULTIMESH;
		return (multimesh_index < static_cast<int>(unordered_associated_multimeshes.size()));
	}

	RID get_multimesh_by_container_and_instance_index(int container_index, int instance_index) {
		auto unordered_associated_multimeshes = get_multimeshes_associated_with_container(container_index);

		// Sort unordered map by RID ID for deterministic ordering
		std::map<uint64_t, RID> ordered_rids; // <RID.get_id(), RID>
		for (const auto &pair : unordered_associated_multimeshes) {
			ordered_rids[pair.first.get_id()] = pair.first;
		}

		// Determine multimesh index based on instance index
		int multimesh_index = instance_index / MAX_MESHES_PER_MULTIMESH;
		int current_index = 0;

		for (const auto &pair : ordered_rids) {
			if (current_index == multimesh_index) {
				return pair.second;
			}
			current_index++;
		}

		return RID(); // Not found, should be handled externally
	}

	void mesh_changed_in_container(int container_index) {
		godot::UtilityFunctions::print("SmartMultiMeshInstance3D::mesh_changed_in_container: Container index " + String::num_int64(container_index));

		for (const auto &m : get_multimeshes_associated_with_container(container_index)) {
			RID mm_rid = m.first;
			RenderingServer *rs = RenderingServer::get_singleton();
			Ref<Mesh> mesh = Object::cast_to<SmartMultiMeshContainer3D>(containers[container_index])->get_mesh();

			rs->multimesh_set_mesh(mm_rid, mesh->get_rid());
		}
	}

	void instance_count_changed_in_container(int container_index) {
		godot::UtilityFunctions::print("SmartMultiMeshInstance3D::instance_count_changed_in_container: Container index " + String::num_int64(container_index));

		Ref<SmartMultiMeshContainer3D> container = Object::cast_to<SmartMultiMeshContainer3D>(containers[container_index]);

		if (container->instance_count == 0) {
			godot::UtilityFunctions::print("SmartMultiMeshInstance3D::instance_count_changed_in_container: Container index " + String::num_int64(container_index) + " has no instances");
			return;
		}

		if (!has_multimesh_for_container_and_instance_index(container_index, container->instance_count)) {
			recreate_multimeshes_and_rs_instances_for_container(container_index);
		} else {
			for (const auto &amm : get_multimeshes_associated_with_container(container_index)) {
				RID mm_rid = amm.first;

				RenderingServer *rs = RenderingServer::get_singleton();
				int instance_count = Object::cast_to<SmartMultiMeshContainer3D>(containers[container_index])->get_instance_count();
				rs->multimesh_allocate_data(mm_rid, instance_count, RenderingServer::MultimeshTransformFormat::MULTIMESH_TRANSFORM_3D, Object::cast_to<SmartMultiMeshContainer3D>(containers[container_index])->use_colors);
				rs->multimesh_set_visible_instances(mm_rid, instance_count);
			}
		}
	}

	void recreate_multimeshes_and_rs_instances_for_container(int container_index) {
		SmartMultiMeshContainer3D *container = Object::cast_to<SmartMultiMeshContainer3D>(containers[container_index]);

		RID scenario_rid = get_world_3d()->get_scenario();
		RenderingServer *rs = RenderingServer::get_singleton();

		for (const auto &m : get_multimeshes_associated_with_container(container_index)) {
			RID mm_rid = m.first;
			rs->free_rid(mm_rid);
			multimeshes.erase(m.first);
		}

		if (container == nullptr) {
			return;
		}

		Ref<Mesh> raw_mesh = container->get_mesh();
		if (raw_mesh == nullptr || raw_mesh.is_null()) {
			return;
		}

		int total_instances = container->get_instance_count();
		int num_multimeshes = (total_instances + MAX_MESHES_PER_MULTIMESH - 1) / MAX_MESHES_PER_MULTIMESH;

		for (int mesh_index = 0; mesh_index < num_multimeshes; mesh_index++) {
			int remaining_instances = total_instances - (mesh_index * MAX_MESHES_PER_MULTIMESH);
			int instance_count = MIN(remaining_instances, MAX_MESHES_PER_MULTIMESH);

			RID mm_rid = rs->multimesh_create();
			rs->multimesh_set_mesh(mm_rid, raw_mesh->get_rid());
			rs->multimesh_allocate_data(mm_rid, instance_count, RenderingServer::MULTIMESH_TRANSFORM_3D, container->use_colors);
			rs->multimesh_set_visible_instances(mm_rid, instance_count);

			// for (int j = 0; j < container->get_instance_count(); j++) {
			// 	Vector3 pos(
			// 			rng->randf_range(-10.0, 10.0),
			// 			rng->randf_range(-10.0, 10.0),
			// 			rng->randf_range(-10.0, 10.0));
			// 	Transform3D xform;
			// 	xform.origin = pos;
			// 	//rs->multimesh_instance_set_transform(mm_rid, j, xform);
			// }

			RID instance_rid = rs->instance_create();
			rs->instance_set_base(instance_rid, mm_rid);
			rs->instance_set_scenario(instance_rid, scenario_rid);
			rs->instance_set_transform(instance_rid, get_global_transform());
			rs->instance_set_visible(instance_rid, true);
			rs->instance_geometry_set_cast_shadows_setting(instance_rid, RenderingServer::SHADOW_CASTING_SETTING_ON);

			// This RID is now mapped to the container index
			multimeshes[mm_rid] = container_index;
			rs_instances.push_back(instance_rid);
		}
	}

	void recreate_multimeshes_and_rs_instances_for_all_containers() {
		// if (!is_inside_tree()) {
		// 	return;
		// }

		RenderingServer *rs = RenderingServer::get_singleton();

		for (const auto &mm : multimeshes) {
			if (mm.first.is_valid())
				rs->free_rid(mm.first);
		}
		multimeshes.clear();

		for (const RID &instance : rs_instances) {
			if (instance.is_valid())
				rs->free_rid(instance);
		}
		rs_instances.clear();

		if (containers.size() == 0) {
			//ERR_PRINT("No containers");
			return;
		}

		// Ref<RandomNumberGenerator> rng;
		// rng.instantiate();
		// rng->randomize();

		for (int container_index = 0; container_index < containers.size(); container_index++) {
			recreate_multimeshes_and_rs_instances_for_container(container_index);
		}
	}

	friend class SmartMultiMeshContainer3D;

public:
	void set_containers(const TypedArray<SmartMultiMeshContainer3D> &p_containers) {
		// set new containers
		containers = p_containers;

		for (int i = 0; i < containers.size(); i++) {
			Ref<SmartMultiMeshContainer3D> container = containers[i];
			if (container.is_valid()) {
				container->set_instance(this);
			}
		}

		godot::UtilityFunctions::print("SmartMultiMeshInstance3D::set_containers");
	}

	TypedArray<SmartMultiMeshContainer3D> get_containers() const { return containers; }

	int get_internal_total_multimeshes_count() const { return static_cast<int>(multimeshes.size()); }

	int get_total_meshes_count() {
		int total_count = 0;

		for (int i = 0; i < containers.size(); i++) {
			SmartMultiMeshContainer3D *container = Object::cast_to<SmartMultiMeshContainer3D>(containers[i]);

			if (container == nullptr)
				continue;

			total_count += container->get_instance_count();
		}
		return total_count;
	}

	void _ready() override {
		/* if (Engine::get_singleton()->is_editor_hint()) {
			return;
		} */

		this->notify_property_list_changed();

		recreate_multimeshes_and_rs_instances_for_all_containers();

		// call gdscript init
		if (has_method("init")) {
			call("init");
		}
	}

	void _exit_tree() override {
		RenderingServer *rs = RenderingServer::get_singleton();

		for (const auto &mm : multimeshes) {
			if (mm.first.is_valid())
				rs->free_rid(mm.first);
		}
		multimeshes.clear();

		for (const RID &instance : rs_instances) {
			if (instance.is_valid())
				rs->free_rid(instance);
		}
		rs_instances.clear();
	}

	int get_container_index(SmartMultiMeshContainer3D *container) const { return static_cast<int>(containers.find(container)); }

	void set_instance_color_by_container_and_instance_index(int container_index, int instance_index, const Color &color) {
		RID multimesh = get_multimesh_by_container_and_instance_index(container_index, instance_index);

		if (!multimesh.is_valid())
			return;

		RenderingServer *rs = RenderingServer::get_singleton();

		int local_index = instance_index % MAX_MESHES_PER_MULTIMESH; // <--- convert to local index
		rs->multimesh_instance_set_color(multimesh, local_index, color);
	}

	void set_instance_transform_by_container_and_instance_index(int container_index, int instance_index, const Transform3D &transform) {
		RID multimesh = get_multimesh_by_container_and_instance_index(container_index, instance_index);

		if (!multimesh.is_valid())
			return;

		RenderingServer *rs = RenderingServer::get_singleton();

		int local_index = instance_index % MAX_MESHES_PER_MULTIMESH; // <--- convert to local index
		rs->multimesh_instance_set_transform(multimesh, local_index, transform);
	}

	//void set_instance_transform(SmartMultiMeshContainer3D *container, int instance_index, const Transform3D &transform);
};
