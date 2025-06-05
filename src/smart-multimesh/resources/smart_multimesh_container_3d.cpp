#include "smart_multimesh_container_3d.h"

#include "smart_multimesh_instance_3d.h"

int SmartMultiMeshContainer3D::get_index() const {
	return sm_instance->get_container_index(const_cast<SmartMultiMeshContainer3D *>(this));
}

void SmartMultiMeshContainer3D::set_instance_transform(int instance_index, const Transform3D &transform) {
	sm_instance->set_instance_transform_by_container_and_instance_index(this->get_index(), instance_index, transform);
}
