/**************************************************************************/
/*  godot_height_map_ex_shape_3d.h                                        */
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

#ifndef GODOT_HEIGHT_MAP_EX_SHAPE_3D_H
#define GODOT_HEIGHT_MAP_EX_SHAPE_3D_H

#include "servers/physics_3d/godot_shape_3d.h"

struct GodotHeightMapExShape3D : public GodotConcaveShape3D {
	Vector<real_t> heights;
	int width = 0;
	int depth = 0;
	real_t cell_size = 1;
	real_t inv_cell_size = 1;
	Vector3 local_origin;

	// Accelerator.
	struct Range {
		real_t min = 0.0;
		real_t max = 0.0;
	};
	LocalVector<Range> bounds_grid;
	int bounds_grid_width = 0;
	int bounds_grid_depth = 0;

	int chunk_cells = 1;
	real_t chunk_size = 1;
	real_t chunk_size_sqr = 1;
	real_t inv_chunk_size = 1;

	static constexpr int BOUNDS_CHUNK_SIZE = 16;

	_FORCE_INLINE_ const Range &_get_bounds_chunk(int p_x, int p_z) const {
		return bounds_grid[(p_z * bounds_grid_width) + p_x];
	}

	_FORCE_INLINE_ real_t _get_height(int p_x, int p_z) const {
		return heights[(p_z * width) + p_x];
	}

	_FORCE_INLINE_ void _get_point(int p_x, int p_z, Vector3 &r_point) const {
		r_point.x = static_cast<real_t>(p_x) - static_cast<real_t>(0.5) * static_cast<real_t>(width - 1);
		r_point.y = _get_height(p_x, p_z);
		r_point.z = static_cast<real_t>(p_z) - static_cast<real_t>(0.5) * static_cast<real_t>(depth - 1);

		r_point.x *= cell_size;
		r_point.z *= cell_size;
	}

	void _get_cell(const Vector3 &p_point, int &r_x, int &r_y, int &r_z) const;

	void _build_accelerator();

	template <typename ProcessFunction>
	bool _intersect_grid_segment(ProcessFunction &p_process, const Vector3 &p_begin, const Vector3 &p_end, int p_width, int p_depth, const Vector3 &offset, Vector3 &r_point, Vector3 &r_normal) const;

	void _setup(const Vector<real_t> &p_heights, int p_width, int p_depth, real_t p_cell_size, real_t p_min_height, real_t p_max_height);

public:
	int get_width() const { return width; }
	int get_depth() const { return depth; }
	real_t get_cell_size() const { return cell_size; }
	int get_chunk_cells() const { return chunk_cells; }
	real_t get_chunk_size() const { return chunk_size; }

	virtual PhysicsServer3D::ShapeType get_type() const override { return PhysicsServer3D::SHAPE_HEIGHTMAP_EX; }

	virtual void project_range(const Vector3 &p_normal, const Transform3D &p_transform, real_t &r_min, real_t &r_max) const override;
	virtual Vector3 get_support(const Vector3 &p_normal) const override;
	virtual bool intersect_segment(const Vector3 &p_begin, const Vector3 &p_end, Vector3 &r_point, Vector3 &r_normal, int &r_face_index, bool p_hit_back_faces) const override;
	virtual bool intersect_point(const Vector3 &p_point) const override;

	virtual Vector3 get_closest_point_to(const Vector3 &p_point) const override;
	virtual void cull(const AABB &p_local_aabb, QueryCallback p_callback, void *p_userdata, bool p_invert_backface_collision) const override;

	virtual Vector3 get_moment_of_inertia(real_t p_mass) const override;

	virtual void set_data(const Variant &p_data) override;
	virtual Variant get_data() const override;

	GodotHeightMapExShape3D();
};

#endif // GODOT_HEIGHT_MAP_EX_SHAPE_3D_H
