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

	std::unordered_map<RID, int> get_multimeshes_associated_with_container(int container_index);

	bool has_multimesh_for_container_and_instance_index(int container_index, int instance_index);
	RID get_multimesh_by_container_and_instance_index(int container_index, int instance_index);

	void mesh_changed_in_container(int container_index);
	void instance_count_changed_in_container(int container_index);
	void recreate_multimeshes_and_rs_instances_for_container(int container_index);
	void recreate_multimeshes_and_rs_instances_for_all_containers();

	friend class SmartMultiMeshContainer3D;

public:
	TypedArray<SmartMultiMeshContainer3D> get_containers() const { return containers; }

	void _ready() override;
	void _exit_tree() override;

	void set_containers(const TypedArray<SmartMultiMeshContainer3D> &p_containers);
	void set_instance_color_by_container_and_instance_index(int container_index, int instance_index, const Color &color);
	void set_instance_transform_by_container_and_instance_index(int container_index, int instance_index, const Transform3D &transform);

	int get_internal_total_multimeshes_count() const { return static_cast<int>(multimeshes.size()); }
	int get_total_meshes_count();
	int get_container_index(SmartMultiMeshContainer3D *container) const { return static_cast<int>(containers.find(container)); }

	Color get_instance_color_by_container_and_instance_index(int container_index, int instance_index);
	Transform3D get_instance_transform_by_container_and_instance_index(int container_index, int instance_index);

	//void set_instance_transform(SmartMultiMeshContainer3D *container, int instance_index, const Transform3D &transform);
};
