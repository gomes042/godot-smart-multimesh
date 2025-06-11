#include "smart_multimesh_container_3d.h"
#include "../smart_multimesh_instance_3d.h"

int SmartMultiMeshContainer3D::get_index() const {
	return sm_instance->get_container_index(const_cast<SmartMultiMeshContainer3D *>(this));
}

void SmartMultiMeshContainer3D::set_instance_transform(int instance_index, const Transform3D &transform) {
	sm_instance->set_instance_transform_by_container_and_instance_index(this->get_index(), instance_index, transform);
}

Transform3D SmartMultiMeshContainer3D::get_instance_transform(int instance_index) const {
	return sm_instance->get_instance_transform_by_container_and_instance_index(this->get_index(), instance_index);
}

Color SmartMultiMeshContainer3D::get_instance_color(int instance_index) {
	return sm_instance->get_instance_color_by_container_and_instance_index(this->get_index(), instance_index);
}

void SmartMultiMeshContainer3D::set_instance_color(int instance_index, const Color &color) {
	sm_instance->set_instance_color_by_container_and_instance_index(this->get_index(), instance_index, color);
}

void SmartMultiMeshContainer3D::set_mesh(const Ref<Mesh> &p_mesh) {
	mesh = p_mesh;

	if (sm_instance == nullptr)
		return;

	sm_instance->mesh_changed_in_container(get_index());
}

void SmartMultiMeshContainer3D::set_instance_count(int count) {
	instance_count = count;

	if (sm_instance == nullptr)
		return;

	sm_instance->instance_count_changed_in_container(get_index());
}

void SmartMultiMeshContainer3D::set_use_indirect(bool p_use_indirect) {
	use_indirect = p_use_indirect;

	notify_property_list_changed();

	if (sm_instance == nullptr)
		return;

	//...
}

void SmartMultiMeshContainer3D::set_transform_format(godot::RenderingServer::MultimeshTransformFormat p_format) {
	transform_format = p_format;

	if (sm_instance == nullptr)
		return;

	//...
}

void SmartMultiMeshContainer3D::set_use_colors(bool p_use_colors) {
	use_colors = p_use_colors;

	if (sm_instance == nullptr)
		return;

	//...
}

void SmartMultiMeshContainer3D::set_use_custom_data(bool p_use_custom_data) {
	use_custom_data = p_use_custom_data;

	if (sm_instance == nullptr)
		return;

	//...
}
