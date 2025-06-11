#pragma once

#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/mesh.hpp"
#include "godot_cpp/classes/rendering_server.hpp"
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/typed_array.hpp"

using namespace godot;

class SmartMultiMeshInstance3D;

class SmartMultiMeshContainer3D : public Resource {
	GDCLASS(SmartMultiMeshContainer3D, Resource);

private:
	Ref<Mesh> mesh;
	int instance_count = 0;

	bool use_indirect = false;
	godot::RenderingServer::MultimeshTransformFormat transform_format = RenderingServer::MULTIMESH_TRANSFORM_3D;
	bool use_colors = false;
	bool use_custom_data = false;

	SmartMultiMeshInstance3D *sm_instance = nullptr;

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

		// METHODS
		ClassDB::bind_method(D_METHOD("get_index"), &SmartMultiMeshContainer3D::get_index);
		ClassDB::bind_method(D_METHOD("set_instance_transform", "instance_index", "transform"), &SmartMultiMeshContainer3D::set_instance_transform);
		//ClassDB::bind_method(D_METHOD("for_every_instance", "callback"), &SmartMultiMeshContainer3D::for_every_instance);
	}

	void set_instance(SmartMultiMeshInstance3D *p_instance) {
		sm_instance = p_instance;
	}
	friend class SmartMultiMeshInstance3D;

public:
	void _get_property_list(List<PropertyInfo> *p_list) const {
		if (!use_indirect) {
			p_list->push_back(PropertyInfo(Variant::INT, "transform_format", PROPERTY_HINT_ENUM, "2D,3D"));
		}
	}

	// Mesh
	void set_mesh(const Ref<Mesh> &p_mesh);
	Ref<Mesh> get_mesh() const { return mesh; }

	// Instance Count
	void set_instance_count(int count);
	int get_instance_count() const { return instance_count; }

	// Use Indirect
	void set_use_indirect(bool p_use_indirect);
	bool get_use_indirect() const { return use_indirect; }

	// Transform Format
	void set_transform_format(godot::RenderingServer::MultimeshTransformFormat p_format);
	godot::RenderingServer::MultimeshTransformFormat get_transform_format() const { return transform_format; }

	// Use Colors
	void set_use_colors(bool p_use_colors);
	bool get_use_colors() const { return use_colors; }

	// Use Custom Data
	void set_use_custom_data(bool p_use_custom_data);
	bool get_use_custom_data() const { return use_custom_data; }

	/* void for_every_instance(const Callable &callback) const {
		for (int i = 0; i < instance_count; i++) {
			Variant instance_data = i;

			callback.callv(godot::Array::make(instance_data));
		}
	} */

	int get_index() const;

	void set_instance_transform(int instance_index, const Transform3D &transform);
	Transform3D get_instance_transform(int instance_index) const;

	void set_instance_color(int instance_index, const Color &color);
	Color get_instance_color(int instance_index);
};
