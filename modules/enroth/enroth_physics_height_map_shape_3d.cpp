#include "enroth_physics_height_map_shape_3d.h"

#include "core/io/image.h"

// EnrothPhysicsHeightMapShape3D is based on GodotHeightMapShape3D

namespace {

struct _HeightmapSegmentCullParams {
	Vector3 from;
	Vector3 to;
	Vector3 dir;

	Vector3 result;
	Vector3 normal;

	const EnrothPhysicsHeightMapShape3D *heightmap = nullptr;
	GodotFaceShape3D *face = nullptr;
};

struct _HeightmapGridCullState {
	real_t length = 0.0;
	real_t length_flat = 0.0;

	real_t dist = 0.0;
	real_t prev_dist = 0.0;

	int x = 0;
	int z = 0;
};

_FORCE_INLINE_ bool _heightmap_face_cull_segment(_HeightmapSegmentCullParams &p_params) {
	Vector3 res;
	Vector3 normal;
	int fi = -1;
	if (p_params.face->intersect_segment(p_params.from, p_params.to, res, normal, fi, true)) {
		p_params.result = res;
		p_params.normal = normal;

		return true;
	}

	return false;
}

_FORCE_INLINE_ bool _heightmap_cell_cull_segment(_HeightmapSegmentCullParams &p_params, const _HeightmapGridCullState &p_state) {
	// First triangle.
	p_params.heightmap->_get_point(p_state.x, p_state.z, p_params.face->vertex[0]);
	p_params.heightmap->_get_point(p_state.x + 1, p_state.z, p_params.face->vertex[1]);
	p_params.heightmap->_get_point(p_state.x + 1, p_state.z + 1, p_params.face->vertex[2]);
	p_params.face->normal = Plane(p_params.face->vertex[0], p_params.face->vertex[1], p_params.face->vertex[2]).normal;
	if (_heightmap_face_cull_segment(p_params)) {
		return true;
	}

	// Second triangle.
	SWAP(p_params.face->vertex[0], p_params.face->vertex[2]);
	p_params.heightmap->_get_point(p_state.x, p_state.z + 1, p_params.face->vertex[1]);
	p_params.face->normal = Plane(p_params.face->vertex[0], p_params.face->vertex[1], p_params.face->vertex[2]).normal;
	if (_heightmap_face_cull_segment(p_params)) {
		return true;
	}

	return false;
}

_FORCE_INLINE_ bool _heightmap_chunk_cull_segment(_HeightmapSegmentCullParams &p_params, const _HeightmapGridCullState &p_state) {
	const EnrothPhysicsHeightMapShape3D::Range &chunk = p_params.heightmap->_get_bounds_chunk(p_state.x, p_state.z);

	Vector3 enter_pos;
	Vector3 exit_pos;

	if (p_state.length_flat > CMP_EPSILON) {
		real_t flat_to_3d = p_state.length / p_state.length_flat;
		real_t enter_param = p_state.prev_dist * flat_to_3d;
		real_t exit_param = p_state.dist * flat_to_3d;
		enter_pos = p_params.from + p_params.dir * enter_param;
		exit_pos = p_params.from + p_params.dir * exit_param;
	} else {
		// Consider the ray vertical.
		// (though we shouldn't reach this often because there is an early check up-front)
		enter_pos = p_params.from;
		exit_pos = p_params.to;
	}

	// Transform positions to heightmap space.
	enter_pos *= p_params.heightmap->get_chunk_size();
	exit_pos *= p_params.heightmap->get_chunk_size();

	// We did enter the flat projection of the AABB,
	// but we have to check if we intersect it on the vertical axis.
	if ((enter_pos.y > chunk.max) && (exit_pos.y > chunk.max)) {
		return false;
	}
	if ((enter_pos.y < chunk.min) && (exit_pos.y < chunk.min)) {
		return false;
	}

	return p_params.heightmap->_intersect_grid_segment(_heightmap_cell_cull_segment, enter_pos, exit_pos, p_params.heightmap->width, p_params.heightmap->depth, p_params.heightmap->local_origin, p_params.result, p_params.normal);
}

};


void EnrothPhysicsHeightMapShape3D::project_range(const Vector3 &p_normal, const Transform3D &p_transform, real_t &r_min, real_t &r_max) const {
	//not very useful, but not very used either
	p_transform.xform(get_aabb()).project_range_in_plane(Plane(p_normal), r_min, r_max);
}

Vector3 EnrothPhysicsHeightMapShape3D::get_support(const Vector3 &p_normal) const {
	//not very useful, but not very used either
	return get_aabb().get_support(p_normal);
}

template <typename ProcessFunction>
bool EnrothPhysicsHeightMapShape3D::_intersect_grid_segment(ProcessFunction &p_process, const Vector3 &p_begin, const Vector3 &p_end, int p_width, int p_depth, const Vector3 &offset, Vector3 &r_point, Vector3 &r_normal) const {
	Vector3 delta = (p_end - p_begin);
	real_t length = delta.length();

	if (length < CMP_EPSILON) {
		return false;
	}

	Vector3 local_begin = p_begin + offset;

	local_begin *= inv_cell_size;

	GodotFaceShape3D face;
	face.backface_collision = false;

	_HeightmapSegmentCullParams params;
	params.from = p_begin;
	params.to = p_end;
	params.dir = delta / length;
	params.heightmap = this;
	params.face = &face;

	_HeightmapGridCullState state;

	// Perform grid query from projected ray.
	Vector2 ray_dir_flat(delta.x, delta.z);
	state.length = length;
	state.length_flat = ray_dir_flat.length();

	if (state.length_flat < CMP_EPSILON) {
		ray_dir_flat = Vector2();
	} else {
		ray_dir_flat /= state.length_flat;
	}

	const int x_step = (ray_dir_flat.x > CMP_EPSILON) ? 1 : ((ray_dir_flat.x < -CMP_EPSILON) ? -1 : 0);
	const int z_step = (ray_dir_flat.y > CMP_EPSILON) ? 1 : ((ray_dir_flat.y < -CMP_EPSILON) ? -1 : 0);

	const real_t infinite = 1e20;
	const real_t delta_x = (x_step != 0) ? 1.f / Math::abs(ray_dir_flat.x) : infinite;
	const real_t delta_z = (z_step != 0) ? 1.f / Math::abs(ray_dir_flat.y) : infinite;

	real_t cross_x; // At which value of `param` we will cross a x-axis lane?
	real_t cross_z; // At which value of `param` we will cross a z-axis lane?

	// X initialization.
	if (x_step != 0) {
		if (x_step == 1) {
			cross_x = (Math::ceil(local_begin.x) - local_begin.x) * delta_x;
		} else {
			cross_x = (local_begin.x - Math::floor(local_begin.x)) * delta_x;
		}
	} else {
		cross_x = infinite; // Will never cross on X.
	}

	// Z initialization.
	if (z_step != 0) {
		if (z_step == 1) {
			cross_z = (Math::ceil(local_begin.z) - local_begin.z) * delta_z;
		} else {
			cross_z = (local_begin.z - Math::floor(local_begin.z)) * delta_z;
		}
	} else {
		cross_z = infinite; // Will never cross on Z.
	}

	int x = Math::floor(local_begin.x);
	int z = Math::floor(local_begin.z);

	// Workaround cases where the ray starts at an integer position.
	if (Math::is_zero_approx(cross_x)) {
		cross_x += delta_x;
		// If going backwards, we should ignore the position we would get by the above flooring,
		// because the ray is not heading in that direction.
		if (x_step == -1) {
			x -= 1;
		}
	}

	if (Math::is_zero_approx(cross_z)) {
		cross_z += delta_z;
		if (z_step == -1) {
			z -= 1;
		}
	}

	// Start inside the grid.
	int x_start = MAX(MIN(x, p_width - 2), 0);
	int z_start = MAX(MIN(z, p_depth - 2), 0);

	// Adjust initial cross values.
	cross_x += delta_x * x_step * (x_start - x);
	cross_z += delta_z * z_step * (z_start - z);

	x = x_start;
	z = z_start;

	while (true) {
		state.prev_dist = state.dist;
		state.x = x;
		state.z = z;

		if (cross_x < cross_z) {
			// X lane.
			x += x_step;
			// Assign before advancing the param,
			// to be in sync with the initialization step.
			state.dist = cross_x;
			cross_x += delta_x;
		} else {
			// Z lane.
			z += z_step;
			state.dist = cross_z;
			cross_z += delta_z;
		}

		if (state.dist > state.length_flat) {
			state.dist = state.length_flat;
			if (p_process(params, state)) {
				r_point = params.result;
				r_normal = params.normal;
				return true;
			}
			break;
		}

		if (p_process(params, state)) {
			r_point = params.result;
			r_normal = params.normal;
			return true;
		}

		// Stop when outside the grid.
		if ((x < 0) || (z < 0) || (x >= p_width - 1) || (z >= p_depth - 1)) {
			break;
		}
	}

	return false;
}

bool EnrothPhysicsHeightMapShape3D::intersect_segment(const Vector3 &p_begin, const Vector3 &p_end, Vector3 &r_point, Vector3 &r_normal, int &r_face_index, bool p_hit_back_faces) const {
	if (heights.is_empty()) {
		return false;
	}

	Vector3 local_begin = p_begin + local_origin;
	Vector3 local_end = p_end + local_origin;

	local_begin *= inv_cell_size;
	local_end *= inv_cell_size;

	// Quantize the ray begin/end.
	const int begin_x = Math::floor(local_begin.x);
	const int begin_z = Math::floor(local_begin.z);
	const int end_x = Math::floor(local_end.x);
	const int end_z = Math::floor(local_end.z);

	if ((begin_x == end_x) && (begin_z == end_z)) {
		// Simple case for rays that don't traverse the grid horizontally.
		// Just perform a test on the given cell.
		GodotFaceShape3D face;
		face.backface_collision = p_hit_back_faces;

		_HeightmapSegmentCullParams params;
		params.from = p_begin;
		params.to = p_end;
		params.dir = (p_end - p_begin).normalized();

		params.heightmap = this;
		params.face = &face;

		_HeightmapGridCullState state;
		state.x = MAX(MIN(begin_x, width - 2), 0);
		state.z = MAX(MIN(begin_z, depth - 2), 0);
		if (_heightmap_cell_cull_segment(params, state)) {
			r_point = params.result;
			r_normal = params.normal;
			return true;
		}

		return false;
	}

	if (bounds_grid.is_empty()) {
		// Process all cells intersecting the flat projection of the ray.
		return _intersect_grid_segment(_heightmap_cell_cull_segment, p_begin, p_end, width, depth, local_origin, r_point, r_normal);
	}

	const Vector3 ray_diff = (p_end - p_begin);
	const real_t length_flat_sqr = ray_diff.x * ray_diff.x + ray_diff.z * ray_diff.z;
	if (length_flat_sqr < chunk_size_sqr) {
		// Don't use chunks, the ray is too short in the plane.
		return _intersect_grid_segment(_heightmap_cell_cull_segment, p_begin, p_end, width, depth, local_origin, r_point, r_normal);
	}

	{
		// The ray is long, run raycast on a higher-level grid.
		Vector3 bounds_from = p_begin * inv_chunk_size;
		Vector3 bounds_to = p_end * inv_chunk_size;
		Vector3 bounds_offset = local_origin * inv_chunk_size;
		return _intersect_grid_segment(_heightmap_chunk_cull_segment, bounds_from, bounds_to, bounds_grid_width, bounds_grid_depth, bounds_offset, r_point, r_normal);
	}
}

bool EnrothPhysicsHeightMapShape3D::intersect_point(const Vector3 &p_point) const {
	return false;
}

Vector3 EnrothPhysicsHeightMapShape3D::get_closest_point_to(const Vector3 &p_point) const {
	return Vector3();
}

void EnrothPhysicsHeightMapShape3D::_get_cell(const Vector3 &p_point, int &r_x, int &r_y, int &r_z) const {
	const AABB &shape_aabb = get_aabb();

	Vector3 pos_local = shape_aabb.position + local_origin;

	Vector3 clamped_point(p_point);
	clamped_point.x = CLAMP(p_point.x, pos_local.x, pos_local.x + shape_aabb.size.x) * inv_cell_size;
	clamped_point.y = CLAMP(p_point.y, pos_local.y, pos_local.y + shape_aabb.size.y);
	clamped_point.z = CLAMP(p_point.z, pos_local.z, pos_local.z + shape_aabb.size.z) * inv_cell_size;

	r_x = (clamped_point.x < 0.0) ? (clamped_point.x - 0.5) : (clamped_point.x + 0.5);
	r_y = (clamped_point.y < 0.0) ? (clamped_point.y - 0.5) : (clamped_point.y + 0.5);
	r_z = (clamped_point.z < 0.0) ? (clamped_point.z - 0.5) : (clamped_point.z + 0.5);
}

void EnrothPhysicsHeightMapShape3D::cull(const AABB &p_local_aabb, QueryCallback p_callback, void *p_userdata, bool p_invert_backface_collision) const {
	if (heights.is_empty()) {
		return;
	}

	AABB local_aabb = p_local_aabb;
	local_aabb.position += local_origin;

	// Quantize the aabb, and adjust the start/end ranges.
	int aabb_min[3];
	int aabb_max[3];
	_get_cell(local_aabb.position, aabb_min[0], aabb_min[1], aabb_min[2]);
	_get_cell(local_aabb.position + local_aabb.size, aabb_max[0], aabb_max[1], aabb_max[2]);

	// Expand the min/max quantized values.
	// This is to catch the case where the input aabb falls between grid points.
	for (int i = 0; i < 3; ++i) {
		aabb_min[i]--;
		aabb_max[i]++;
	}

	int start_x = MAX(0, aabb_min[0]);
	int end_x = MIN(width - 1, aabb_max[0]);
	int start_z = MAX(0, aabb_min[2]);
	int end_z = MIN(depth - 1, aabb_max[2]);

	GodotFaceShape3D face;
	face.backface_collision = !p_invert_backface_collision;
	face.invert_backface_collision = p_invert_backface_collision;

	for (int z = start_z; z < end_z; z++) {
		for (int x = start_x; x < end_x; x++) {
			// First triangle.
			_get_point(x, z, face.vertex[0]);
			_get_point(x + 1, z, face.vertex[1]);
			_get_point(x + 1, z + 1, face.vertex[2]);
			face.normal = Plane(face.vertex[0], face.vertex[1], face.vertex[2]).normal;
			if (p_callback(p_userdata, &face)) {
				return;
			}

			// Second triangle.
			SWAP(face.vertex[0], face.vertex[2]);
			_get_point(x, z + 1, face.vertex[1]);
			face.normal = Plane(face.vertex[0], face.vertex[1], face.vertex[2]).normal;
			if (p_callback(p_userdata, &face)) {
				return;
			}
		}
	}
}

Vector3 EnrothPhysicsHeightMapShape3D::get_moment_of_inertia(real_t p_mass) const {
	// use bad AABB approximation
	Vector3 extents = get_aabb().size * 0.5;

	return Vector3(
			(p_mass / 3.0) * (extents.y * extents.y + extents.z * extents.z),
			(p_mass / 3.0) * (extents.x * extents.x + extents.z * extents.z),
			(p_mass / 3.0) * (extents.x * extents.x + extents.y * extents.y));
}

void EnrothPhysicsHeightMapShape3D::_build_accelerator() {
	bounds_grid.clear();

	chunk_cells = static_cast<int>(Math::floor(BOUNDS_CHUNK_SIZE * inv_cell_size));
	chunk_size = cell_size * static_cast<real_t>(chunk_cells);
	chunk_size_sqr = chunk_size * chunk_size;
	inv_chunk_size = static_cast<real_t>(1) / chunk_size;

	bounds_grid_width = width / chunk_cells;
	bounds_grid_depth = depth / chunk_cells;

	if (width % chunk_cells > 0) {
		++bounds_grid_width; // In case terrain size isn't dividable by chunk size.
	}

	if (depth % chunk_cells > 0) {
		++bounds_grid_depth;
	}

	const int bound_grid_size = bounds_grid_width * bounds_grid_depth;

	if (bound_grid_size < 2) {
		// Grid is empty or just one chunk.
		return;
	}

	bounds_grid.resize(bound_grid_size);

	// Compute min and max height for all chunks.
	for (int cz = 0; cz < bounds_grid_depth; ++cz) {
		const int z0 = cz * chunk_cells;

		for (int cx = 0; cx < bounds_grid_width; ++cx) {
			const int x0 = cx * chunk_cells;

			Range r;

			r.min = _get_height(x0, z0);
			r.max = r.min;

			// Compute min and max height for this chunk.
			// We have to include one extra cell to account for neighbors.
			// Here is why:
			// Say we have a flat terrain, and a plateau that fits a chunk perfectly.
			//
			//   Left        Right
			// 0---0---0---1---1---1
			// |   |   |   |   |   |
			// 0---0---0---1---1---1
			// |   |   |   |   |   |
			// 0---0---0---1---1---1
			//           x
			//
			// If the AABB for the Left chunk did not share vertices with the Right,
			// then we would fail collision tests at x due to a gap.
			//
			const int z_max = MIN(z0 + chunk_cells + 1, depth);
			const int x_max = MIN(x0 + chunk_cells + 1, width);
			for (int z = z0; z < z_max; ++z) {
				for (int x = x0; x < x_max; ++x) {
					const real_t height = _get_height(x, z);
					if (height < r.min) {
						r.min = height;
					} else if (height > r.max) {
						r.max = height;
					}
				}
			}

			bounds_grid[cz * bounds_grid_width + cx] = r;
		}
	}
}

void EnrothPhysicsHeightMapShape3D::_setup(const Vector<real_t> &p_heights, int p_width, int p_depth, real_t p_cell_size, real_t p_min_height, real_t p_max_height) {
	heights = p_heights;
	width = p_width;
	depth = p_depth;
	cell_size = p_cell_size;
	inv_cell_size = static_cast<real_t>(1) / p_cell_size;

	// Initialize aabb.
	AABB aabb_new;
	aabb_new.position = Vector3(0.0, p_min_height, 0.0);
	aabb_new.size = Vector3(p_width - 1, p_max_height - p_min_height, p_depth - 1);

	aabb_new.size.x *= cell_size;
	aabb_new.size.z *= cell_size;

	// Initialize origin as the aabb center.
	local_origin = aabb_new.position + 0.5 * aabb_new.size;
	local_origin.y = 0.0;

	aabb_new.position -= local_origin;

	_build_accelerator();

	configure(aabb_new);
}

void EnrothPhysicsHeightMapShape3D::set_data(const Variant &p_data) {
	ERR_FAIL_COND(p_data.get_type() != Variant::DICTIONARY);

	Dictionary d = p_data;
	ERR_FAIL_COND(!d.has("width"));
	ERR_FAIL_COND(!d.has("depth"));
	ERR_FAIL_COND(!d.has("heights"));

	const int width_new = d["width"];
	const int depth_new = d["depth"];

	ERR_FAIL_COND(width_new <= 0);
	ERR_FAIL_COND(depth_new <= 0);

	real_t cell_size_new = 1;

	if (d.has("cell_size")) {
		cell_size_new = d["cell_size"];
		ERR_FAIL_COND(cell_size_new < static_cast<real_t>(0.1));
	}

	Variant heights_variant = d["heights"];
	Vector<real_t> heights_buffer;
#ifdef REAL_T_IS_DOUBLE
	if (heights_variant.get_type() == Variant::PACKED_FLOAT64_ARRAY) {
#else
	if (heights_variant.get_type() == Variant::PACKED_FLOAT32_ARRAY) {
#endif
		// Ready-to-use heights can be passed.
		heights_buffer = heights_variant;
	} else if (heights_variant.get_type() == Variant::OBJECT) {
		// If an image is passed, we have to convert it.
		// This would be expensive to do with a script, so it's nice to have it here.
		Ref<Image> image = heights_variant;
		ERR_FAIL_COND(image.is_null());
		ERR_FAIL_COND(image->get_format() != Image::FORMAT_RF);

		PackedByteArray im_data = image->get_data();
		heights_buffer.resize(image->get_width() * image->get_height());

		real_t *w = heights_buffer.ptrw();
		real_t *rp = (real_t *)im_data.ptr();
		for (int i = 0; i < heights_buffer.size(); ++i) {
			w[i] = rp[i];
		}
	} else {
#ifdef REAL_T_IS_DOUBLE
		ERR_FAIL_MSG("Expected PackedFloat64Array or float Image.");
#else
		ERR_FAIL_MSG("Expected PackedFloat32Array or float Image.");
#endif
	}

	// Compute min and max heights or use precomputed values.
	real_t min_height = 0.0;
	real_t max_height = 0.0;
	if (d.has("min_height") && d.has("max_height")) {
		min_height = d["min_height"];
		max_height = d["max_height"];
	} else {
		int heights_size = heights.size();
		for (int i = 0; i < heights_size; ++i) {
			real_t h = heights[i];
			if (h < min_height) {
				min_height = h;
			} else if (h > max_height) {
				max_height = h;
			}
		}
	}

	ERR_FAIL_COND(min_height > max_height);

	ERR_FAIL_COND(heights_buffer.size() != (width_new * depth_new));

	// If specified, min and max height will be used as precomputed values.
	_setup(heights_buffer, width_new, depth_new, cell_size_new, min_height, max_height);
}

Variant EnrothPhysicsHeightMapShape3D::get_data() const {
	Dictionary d;
	d["width"] = width;
	d["depth"] = depth;
	d["cell_size"] = cell_size;

	const AABB &shape_aabb = get_aabb();
	d["min_height"] = shape_aabb.position.y;
	d["max_height"] = shape_aabb.position.y + shape_aabb.size.y;

	d["heights"] = heights;

	return d;
}

EnrothPhysicsHeightMapShape3D::EnrothPhysicsHeightMapShape3D() {
}
