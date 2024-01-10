#pragma once

#include "core/io/file_access.h"

#include "Types/DBCommon.hpp"

class FileAccessBinarySerializer final : public enroth::data::BinarySerializer {
private:
	const Ref<FileAccess> file;
protected:
	std::size_t WriteBytes(const void *const buffer, const std::size_t size) override {
		file->store_buffer(static_cast<const uint8_t *>(buffer), size);
		return size;
	}
public:
	explicit FileAccessBinarySerializer(const Ref<FileAccess> &file) :
		file(file) {
	}
};

class FileAccessBinaryDeserializer final : public enroth::data::BinaryDeserializer {
private:
	const Ref<FileAccess> file;
protected:
	std::size_t ReadBytes(void *const buffer, const std::size_t size) override {
		return file->get_buffer(static_cast<uint8_t *>(buffer), size);
	}
	std::size_t GetAvailableBytes() const override {
		return file->get_length() - file->get_position();
	}
public:
	explicit FileAccessBinaryDeserializer(const Ref<FileAccess> &file) :
		file(file) {
	}
};
