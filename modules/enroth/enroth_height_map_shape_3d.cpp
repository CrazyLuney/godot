#include "enroth_height_map_shape_3d.h"

#include "servers/physics_server_3d.h"

Vector<Vector3> EnrothHeightMapShape3D::get_debug_mesh_lines() const {
	Vector<Vector3> points;

	if ((map_width > 0) && (map_depth > 0)) {
		Vector2 size(static_cast<real_t>(map_width - 1), static_cast<real_t>(map_depth - 1));

		size *= map_cell_size;

		Vector2 start = size * -0.5;

		// reserve some memory for our points..
		points.resize(((map_width - 1) * map_depth * 2) + (map_width * (map_depth - 1) * 2) + ((map_width - 1) * (map_depth - 1) * 2));

		// now set our points
		auto w = points.ptrw();

		for (int z = 0; z < map_depth; z++) {
			Vector3 point(start.x, 0.0, start.y);

			for (int x = 0; x < map_width; x++) {
				point.y = _get_height(x, z);

				if (x != map_width - 1) {
					*w++ = point;
					*w++ = Vector3(point.x + map_cell_size, _get_height(x + 1, z), point.z);
				}

				if (z != map_depth - 1) {
					*w++ = point;
					*w++ = Vector3(point.x, _get_height(x, z + 1), point.z + map_cell_size);
				}

				if ((x != map_width - 1) && (z != map_depth - 1)) {
					*w++ = point;
					*w++ = Vector3(point.x + map_cell_size, _get_height(x + 1, z + 1), point.z + map_cell_size);
				}

				point.x += map_cell_size;
			}

			start.y += map_cell_size;
		}
	}

	return points;
}

real_t EnrothHeightMapShape3D::get_enclosing_radius() const {
	return Vector3(static_cast<real_t>(map_width) * map_cell_size, max_height - min_height, static_cast<real_t>(map_depth) * map_cell_size).length();
}

void EnrothHeightMapShape3D::_update_shape() {
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

void EnrothHeightMapShape3D::set_map_width(int p_new) {
	if (p_new < 1)
		return;
	if (p_new == map_width)
		return;

	const int was_size = map_width * map_depth;
	map_width = p_new;

	const int new_size = map_width * map_depth;
	map_data.resize(new_size);

	const auto w = map_data.ptrw();

	for (auto i = was_size; i < new_size; ++i) {
		w[i] = 0;
	}

	_update_shape();
	emit_changed();
}

void EnrothHeightMapShape3D::set_map_depth(int p_new) {
	if (p_new < 1)
		return;
	if (p_new == map_depth)
		return;

	const int was_size = map_width * map_depth;
	map_depth = p_new;

	const int new_size = map_width * map_depth;
	map_data.resize(new_size);

	const auto w = map_data.ptrw();

	for (auto i = was_size; i < new_size; ++i) {
		w[i] = 0;
	}

	_update_shape();
	emit_changed();
}

void EnrothHeightMapShape3D::set_map_cell_size(real_t p_new) {
	if (p_new < static_cast<real_t>(0.1))
		return;
	if (Math::is_equal_approx(p_new, map_cell_size, static_cast<real_t>(CMP_EPSILON)))
		return;

	map_cell_size = p_new;

	_update_shape();
	emit_changed();
}

void EnrothHeightMapShape3D::set_map_height_scale(real_t p_new) {
	if (p_new < static_cast<real_t>(0.001))
		return;
	if (Math::is_equal_approx(p_new, map_height_scale, static_cast<real_t>(CMP_EPSILON)))
		return;

	map_height_scale = p_new;

	_update_shape();
	emit_changed();
}

void EnrothHeightMapShape3D::set_map_data(const PackedByteArray &p_new) {
	const int size = (map_width * map_depth);

	if (p_new.size() != size)
		return;

	// copy
	const auto r = p_new.ptr();
	const auto w = map_data.ptrw();

	min_height = max_height = static_cast<real_t>(r[0]) * map_height_scale;
	
	for (int i = 0; i < size; i++) {
		w[i] = r[i];

		const real_t height = static_cast<real_t>(r[i]) * map_height_scale;

		if (min_height > height)
			min_height = height;
		if (max_height < height)
			max_height = height;
	}

	_update_shape();
	emit_changed();
}

void EnrothHeightMapShape3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_map_width", "width"), &EnrothHeightMapShape3D::set_map_width);
	ClassDB::bind_method(D_METHOD("get_map_width"), &EnrothHeightMapShape3D::get_map_width);
	ClassDB::bind_method(D_METHOD("set_map_depth", "height"), &EnrothHeightMapShape3D::set_map_depth);
	ClassDB::bind_method(D_METHOD("get_map_depth"), &EnrothHeightMapShape3D::get_map_depth);
	ClassDB::bind_method(D_METHOD("set_map_cell_size", "cell_size"), &EnrothHeightMapShape3D::set_map_cell_size);
	ClassDB::bind_method(D_METHOD("get_map_cell_size"), &EnrothHeightMapShape3D::get_map_cell_size);
	ClassDB::bind_method(D_METHOD("set_map_height_scale", "scale"), &EnrothHeightMapShape3D::set_map_height_scale);
	ClassDB::bind_method(D_METHOD("get_map_height_scale"), &EnrothHeightMapShape3D::get_map_height_scale);
	ClassDB::bind_method(D_METHOD("set_map_data", "data"), &EnrothHeightMapShape3D::set_map_data);
	ClassDB::bind_method(D_METHOD("get_map_data"), &EnrothHeightMapShape3D::get_map_data);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "map_width", PROPERTY_HINT_RANGE, "1,1024,1,or_greater"), "set_map_width", "get_map_width");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "map_depth", PROPERTY_HINT_RANGE, "1,1024,1,or_greater"), "set_map_depth", "get_map_depth");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "map_cell_size", PROPERTY_HINT_RANGE, "0,1000,1,or_greater"), "set_map_cell_size", "get_map_cell_size");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "map_height_scale", PROPERTY_HINT_RANGE, "0,1000,0.1,or_greater"), "set_map_height_scale", "get_map_height_scale");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "map_data"), "set_map_data", "get_map_data");
}

EnrothHeightMapShape3D::EnrothHeightMapShape3D() :
		Shape3D(PhysicsServer3D::get_singleton()->shape_create(PhysicsServer3D::SHAPE_HEIGHTMAP_EX)) {
	map_data.resize(map_width * map_depth);
	map_data.fill(0);

	_update_shape();
}
