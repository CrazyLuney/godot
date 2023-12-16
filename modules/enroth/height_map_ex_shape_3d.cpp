/**************************************************************************/
/*  height_map_ex_shape_3d.cpp                                            */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "height_map_ex_shape_3d.h"

#include "servers/physics_server_3d.h"

Vector<Vector3> HeightMapExShape3D::get_debug_mesh_lines() const {
	Vector<Vector3> points;

	if ((map_width != 0) && (map_depth != 0)) {
		// This will be slow for large maps...
		// also we'll have to figure out how well bullet centers this shape...

		Vector2 size(map_width - 1, map_depth - 1);

		size *= map_cell_size;

		Vector2 start = size * -0.5;

		const real_t *r = map_data.ptr();

		// reserve some memory for our points..
		points.resize(((map_width - 1) * map_depth * 2) + (map_width * (map_depth - 1) * 2) + ((map_width - 1) * (map_depth - 1) * 2));

		// now set our points
		int r_offset = 0;
		int w_offset = 0;
		for (int d = 0; d < map_depth; d++) {
			Vector3 height(start.x, 0.0, start.y);

			for (int w = 0; w < map_width; w++) {
				height.y = r[r_offset++];

				if (w != map_width - 1) {
					points.write[w_offset++] = height;
					points.write[w_offset++] = Vector3(height.x + map_cell_size, r[r_offset], height.z);
				}

				if (d != map_depth - 1) {
					points.write[w_offset++] = height;
					points.write[w_offset++] = Vector3(height.x, r[r_offset + map_width - 1], height.z + map_cell_size);
				}

				if ((w != map_width - 1) && (d != map_depth - 1)) {
					points.write[w_offset++] = height;
					points.write[w_offset++] = Vector3(height.x + map_cell_size, r[r_offset + map_width], height.z + map_cell_size);
				}

				height.x += map_cell_size;
			}

			start.y += map_cell_size;
		}
	}

	return points;
}

real_t HeightMapExShape3D::get_enclosing_radius() const {
	return Vector3(static_cast<real_t>(map_width) * map_cell_size, max_height - min_height, static_cast<real_t>(map_depth) * map_cell_size).length();
}

void HeightMapExShape3D::_update_shape() {
	Dictionary d;
	d["width"] = map_width;
	d["depth"] = map_depth;
	d["cell_size"] = map_cell_size;
	d["heights"] = map_data;
	d["min_height"] = min_height;
	d["max_height"] = max_height;
	PhysicsServer3D::get_singleton()->shape_set_data(get_shape(), d);
	Shape3D::_update_shape();
}

void HeightMapExShape3D::set_map_width(int p_new) {
	if (p_new < 1) {
		// ignore
	} else if (map_width != p_new) {
		int was_size = map_width * map_depth;
		map_width = p_new;

		int new_size = map_width * map_depth;
		map_data.resize(map_width * map_depth);

		real_t *w = map_data.ptrw();
		while (was_size < new_size) {
			w[was_size++] = 0.0;
		}

		_update_shape();
		emit_changed();
	}
}

int HeightMapExShape3D::get_map_width() const {
	return map_width;
}

void HeightMapExShape3D::set_map_depth(int p_new) {
	if (p_new < 1) {
		// ignore
	} else if (map_depth != p_new) {
		int was_size = map_width * map_depth;
		map_depth = p_new;

		int new_size = map_width * map_depth;
		map_data.resize(new_size);

		real_t *w = map_data.ptrw();
		while (was_size < new_size) {
			w[was_size++] = 0.0;
		}

		_update_shape();
		emit_changed();
	}
}

int HeightMapExShape3D::get_map_depth() const {
	return map_depth;
}

void HeightMapExShape3D::set_map_cell_size(real_t p_new) {
	if (p_new < static_cast<real_t>(0.1))
		// ignore
		return;
	if (Math::is_equal_approx(map_cell_size, p_new, static_cast<real_t>(CMP_EPSILON)))
		// ignore
		return;

	map_cell_size = p_new;

	_update_shape();
	emit_changed();
}

real_t HeightMapExShape3D::get_map_cell_size() const {
	return map_cell_size;
}

void HeightMapExShape3D::set_map_data(Vector<real_t> p_new) {
	int size = (map_width * map_depth);
	if (p_new.size() != size) {
		// fail
		return;
	}

	// copy
	real_t *w = map_data.ptrw();
	const real_t *r = p_new.ptr();
	for (int i = 0; i < size; i++) {
		real_t val = r[i];
		w[i] = val;
		if (i == 0) {
			min_height = val;
			max_height = val;
		} else {
			if (min_height > val) {
				min_height = val;
			}

			if (max_height < val) {
				max_height = val;
			}
		}
	}

	_update_shape();
	emit_changed();
}

Vector<real_t> HeightMapExShape3D::get_map_data() const {
	return map_data;
}

void HeightMapExShape3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_map_width", "width"), &HeightMapExShape3D::set_map_width);
	ClassDB::bind_method(D_METHOD("get_map_width"), &HeightMapExShape3D::get_map_width);
	ClassDB::bind_method(D_METHOD("set_map_depth", "height"), &HeightMapExShape3D::set_map_depth);
	ClassDB::bind_method(D_METHOD("get_map_depth"), &HeightMapExShape3D::get_map_depth);
	ClassDB::bind_method(D_METHOD("set_map_cell_size", "cell_size"), &HeightMapExShape3D::set_map_cell_size);
	ClassDB::bind_method(D_METHOD("get_map_cell_size"), &HeightMapExShape3D::get_map_cell_size);
	ClassDB::bind_method(D_METHOD("set_map_data", "data"), &HeightMapExShape3D::set_map_data);
	ClassDB::bind_method(D_METHOD("get_map_data"), &HeightMapExShape3D::get_map_data);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "map_width", PROPERTY_HINT_RANGE, "1,1024,1,or_greater"), "set_map_width", "get_map_width");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "map_depth", PROPERTY_HINT_RANGE, "1,1024,1,or_greater"), "set_map_depth", "get_map_depth");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "map_cell_size", PROPERTY_HINT_RANGE, "0,1000,1,or_greater"), "set_map_cell_size", "get_map_cell_size");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT32_ARRAY, "map_data"), "set_map_data", "get_map_data");
}

HeightMapExShape3D::HeightMapExShape3D() :
		Shape3D(PhysicsServer3D::get_singleton()->shape_create(PhysicsServer3D::SHAPE_HEIGHTMAP_EX)) {
	map_data.resize(map_width * map_depth);
	map_data.fill(0);

	_update_shape();
}
