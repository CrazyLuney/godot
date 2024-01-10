#pragma once

#include "scene/resources/shape_3d.h"

class EnrothHeightMapShape3D : public Shape3D {
	GDCLASS(EnrothHeightMapShape3D, Shape3D)

	int map_width = 128;
	int map_depth = 128;
	real_t map_cell_size = 1;
	real_t map_height_scale = 1;
	PackedByteArray map_data;
	real_t min_height = 0.0;
	real_t max_height = 0.0;

	real_t _get_height(int x, int z) const {
		// ERR_FAIL_COND_V(x < 0, 0);
		// ERR_FAIL_COND_V(z < 0, 0);
		// ERR_FAIL_COND_V(x >= map_width, 0);
		// ERR_FAIL_COND_V(z >= map_depth, 0);
		return static_cast<real_t>(map_data[z * map_width + x]) * map_height_scale;
	}

protected:
	static void _bind_methods();

	virtual void _update_shape() override;

public:
	void set_map_width(int p_new);
	int get_map_width() const { return map_width; }
	void set_map_depth(int p_new);
	int get_map_depth() const { return map_depth; }
	void set_map_cell_size(real_t p_new);
	real_t get_map_cell_size() const { return map_cell_size; }
	void set_map_height_scale(real_t p_new);
	real_t get_map_height_scale() const { return map_height_scale; }
	void set_map_data(const PackedByteArray &p_new);
	const PackedByteArray& get_map_data() const { return map_data; }

	virtual Vector<Vector3> get_debug_mesh_lines() const override;
	virtual real_t get_enclosing_radius() const override;

	EnrothHeightMapShape3D();
};
