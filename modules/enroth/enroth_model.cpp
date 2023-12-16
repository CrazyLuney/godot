#include "enroth_model.h"
#include "enroth_helper.h"

namespace {

const String l_enrothModelExtension = "emodel";
const String l_enrothModelClassName = "EnrothModel";

}


void EnrothModel::_bind_methods() {
	// ClassDB::bind_method(D_METHOD("set_header", "header"), &EnrothModel::set_header);
	// ClassDB::bind_method(D_METHOD("get_header"), &EnrothModel::get_header);
	// ClassDB::bind_method(D_METHOD("set_geometry", "geometry"), &EnrothModel::set_geometry);
	// ClassDB::bind_method(D_METHOD("get_geometry"), &EnrothModel::get_geometry);
}


Error EnrothModelSaver::_save_file(const String &p_path, const EnrothModel &p_model) {
	Error error;

	const auto file = FileAccess::open(p_path, FileAccess::WRITE, &error);

	if (error != OK) {
		if (file.is_valid())
			file->close();
		return error;
	}

	ERR_FAIL_COND_V(p_model.geometry.Vertices.size() == p_model.header.NumVertices, ERR_INVALID_DATA);
	ERR_FAIL_COND_V(p_model.geometry.Faces.size() == p_model.header.NumFaces, ERR_INVALID_DATA);
	ERR_FAIL_COND_V(p_model.geometry.FaceOrdering.size() == p_model.header.NumFaces, ERR_INVALID_DATA);
	ERR_FAIL_COND_V(p_model.geometry.Nodes.size() == p_model.header.NumNodes, ERR_INVALID_DATA);
	ERR_FAIL_COND_V(p_model.geometry.FaceTextures.size() == p_model.header.NumFaces, ERR_INVALID_DATA);

	{
		FileAccessBinarySerializer stream { file };

		stream.Write(p_model.header);
		stream.Write(p_model.geometry.Vertices, p_model.header.NumVertices);
		stream.Write(p_model.geometry.Faces, p_model.header.NumFaces);
		stream.Write(p_model.geometry.FaceOrdering, p_model.header.NumFaces);
		stream.Write(p_model.geometry.Nodes, p_model.header.NumNodes);
		stream.Write(p_model.geometry.FaceTextures, p_model.header.NumFaces);
	}

	file->close();

	return OK;
}

Error EnrothModelSaver::save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags) {
	const Ref<EnrothModel> model = p_resource;

	ERR_FAIL_COND_V(model.is_null(), ERR_INVALID_PARAMETER);

	return _save_file(p_path, *model.ptr());
}

bool EnrothModelSaver::recognize(const Ref<Resource> &p_resource) const {
	ERR_FAIL_NULL_V(cast_to<EnrothModel>(*p_resource), false);
	return true;
}

void EnrothModelSaver::get_recognized_extensions(const Ref<Resource> &p_resource, List<String> *p_extensions) const {
	ERR_FAIL_NULL(p_extensions);

	if (cast_to<EnrothModel>(*p_resource))
		p_extensions->push_back(l_enrothModelExtension);
}


Error EnrothModelLoader::_load_file(const String &p_path, EnrothModel &p_model) {
	Error error;

	const auto file = FileAccess::open(p_path, FileAccess::READ, &error);

	if (error != OK) {
		if (file.is_valid()) {
			file->close();
		}
		return error;
	}

	{
		FileAccessBinaryDeserializer stream { file };

		stream.Read(p_model.header);
		stream.Read(p_model.geometry.Vertices, p_model.header.NumVertices);
		stream.Read(p_model.geometry.Faces, p_model.header.NumFaces);
		stream.Read(p_model.geometry.FaceOrdering, p_model.header.NumFaces);
		stream.Read(p_model.geometry.Nodes, p_model.header.NumNodes);
		stream.Read(p_model.geometry.FaceTextures, p_model.header.NumFaces);
	}

	file->close();

	return OK;
}

Ref<Resource> EnrothModelLoader::load(const String &p_path, const String &p_original_path, Error *r_error, bool p_use_sub_threads, float *r_progress, CacheMode p_cache_mode) {
	Ref<EnrothModel> model = memnew(EnrothModel);

	const Error result = _load_file(p_path, *model.ptr());

	if (r_error)
		*r_error = result;

	return model;
}

void EnrothModelLoader::get_recognized_extensions(List<String> *p_extensions) const {
	ERR_FAIL_NULL(p_extensions);

	if (!p_extensions->find(l_enrothModelExtension)) {
		p_extensions->push_back(l_enrothModelExtension);
	}
}

bool EnrothModelLoader::handles_type(const String &p_type) const {
	return p_type == l_enrothModelClassName;
}

String EnrothModelLoader::get_resource_type(const String &p_path) const {
	if (p_path.get_extension().to_lower() == l_enrothModelExtension)
		return l_enrothModelClassName;
	return {};
}
