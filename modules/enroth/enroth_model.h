#pragma once

#include "core/io/resource.h"
#include "core/io/resource_saver.h"
#include "core/io/resource_loader.h"
#include "scene/resources/texture.h"

#include "enroth/src/Types/DBTypes.hpp"

class EnrothModel : public Resource {
	GDCLASS(EnrothModel, Resource)

	friend class EnrothModelSaver;
	friend class EnrothModelLoader;

	enroth::data::mm7::BSPModelHeader header;
	enroth::data::mm7::BSPModelGeometry geometry;

	Vector<Ref<Texture2D>> texture_references;

protected:
	static void _bind_methods();

public:
	void set_header(const enroth::data::mm7::BSPModelHeader& p_new) { header = p_new; }
	const auto &get_header() const { return header; }
	void set_geometry(const enroth::data::mm7::BSPModelGeometry &p_new) { geometry = p_new; }
	const auto &get_geometry() const { return geometry; }

};

class EnrothModelSaver : public ResourceFormatSaver {
	GDCLASS(EnrothModelSaver, ResourceFormatSaver)

	static Error _save_file(const String &p_path, const EnrothModel &p_model);

public:
	Error save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags) override;
	bool recognize(const Ref<Resource> &p_resource) const override;
	void get_recognized_extensions(const Ref<Resource> &p_resource, List<String> *p_extensions) const override;

};

class EnrothModelLoader : public ResourceFormatLoader {
	GDCLASS(EnrothModelLoader, ResourceFormatLoader)

	static Error _load_file(const String &p_path, EnrothModel &p_model);

public:
	Ref<Resource> load(const String &p_path, const String &p_original_path, Error *r_error, bool p_use_sub_threads, float *r_progress, CacheMode p_cache_mode) override;
	void get_recognized_extensions(List<String> *p_extensions) const override;
	bool handles_type(const String &p_type) const override;
	String get_resource_type(const String &p_path) const override;

};
