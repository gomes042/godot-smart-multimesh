#pragma once

#include <godot_cpp/classes/node3d.hpp>

using namespace godot;

class SmartMultiMeshInstance3D : public Node3D {
	GDCLASS(SmartMultiMeshInstance3D, Node3D);

protected:
	static void _bind_methods() {};
};
