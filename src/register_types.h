#pragma once

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_module(ModuleInitializationLevel p_level);
void uninitialize_module(ModuleInitializationLevel p_level);
