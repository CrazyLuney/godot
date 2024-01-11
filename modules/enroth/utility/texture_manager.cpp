#include "texture_manager.hpp"

EnrothTextureManager::EnrothTextureManager(enroth::asset_tools::AssetLoader &asset_loader) :
		_asset_loader(asset_loader) {
}

Ref<Texture2D> EnrothTextureManager::GetBitmapTexture(const String &texture_name) const {
	const String name{ TranslateBitmapTextureName(texture_name) };

	const auto lod_bitmap_id = _asset_loader.FindBitmap(name.ascii().ptr());

	ERR_FAIL_COND_V_MSG(!lod_bitmap_id, {}, "Bitmap not found: " + texture_name);

	{
		const auto it = _textures.find({ lod_bitmap_id });
		if (it != _textures.end())
			return it->value;
	}

	return {};
}

Ref<Image> EnrothTextureManager::GetOrCreateBitmapImage(const String &texture_name) {
	const String name{ TranslateBitmapTextureName(texture_name) };

	const auto lod_bitmap_id = _asset_loader.FindBitmap(name.ascii().ptr());

	ERR_FAIL_COND_V_MSG(!lod_bitmap_id, {}, "Bitmap not found: " + texture_name);

	const auto it = _images.find({ lod_bitmap_id });
	if (it != _images.end())
		return it->value;

	const auto lod_bitmap = _asset_loader.LoadBitmap(name.ascii().ptr());

	ERR_FAIL_COND_V_MSG(!lod_bitmap, {}, "Failed to load bitmap: " + texture_name);

	const auto image = EnrothTextureImporter().Import(lod_bitmap);

	ERR_FAIL_COND_V_MSG(image.is_null(), {}, "Failed to create image: " + texture_name);

	_images.insert({ lod_bitmap_id }, image);

	return image;
}

Ref<Texture2D> EnrothTextureManager::GetOrCreateBitmapTexture(const String &texture_name) {
	const String name{ TranslateBitmapTextureName(texture_name) };

	const auto lod_bitmap_id = _asset_loader.FindBitmap(name.ascii().ptr());

	ERR_FAIL_COND_V_MSG(!lod_bitmap_id, {}, "Bitmap not found: " + texture_name);

	const auto it = _textures.find({ lod_bitmap_id });
	if (it != _textures.end())
		return it->value;

	const auto image = GetOrCreateBitmapImage(name);

	ERR_FAIL_COND_V_MSG(image.is_null(), {}, "Failed to get image: " + texture_name);

	const auto texture = ImageTexture::create_from_image(image);

	ERR_FAIL_COND_V_MSG(texture.is_null(), {}, "Failed to create texture: " + texture_name);

	_textures.insert({ lod_bitmap_id }, texture);

	return image;
}

Ref<Material> EnrothTextureManager::GetOrCreateBitmapMaterial(const String &texture_name) {
	const String name{ TranslateBitmapTextureName(texture_name) };

	const auto lod_bitmap_id = _asset_loader.FindBitmap(name.ascii().ptr());

	ERR_FAIL_COND_V_MSG(!lod_bitmap_id, {}, "Bitmap not found: " + texture_name);

	const auto it = _materials.find({ lod_bitmap_id });
	if (it != _materials.end())
		return it->value;

	const auto texture = GetOrCreateBitmapTexture(name);

	ERR_FAIL_COND_V_MSG(texture.is_null(), {}, "Failed to get texture: " + texture_name);

	const auto material = memnew(StandardMaterial3D);

	material->set_texture(BaseMaterial3D::TEXTURE_ALBEDO, texture);
	material->set_flag(BaseMaterial3D::FLAG_USE_TEXTURE_REPEAT, true);

	_materials.insert({ lod_bitmap_id }, material);

	return material;
}

Ref<Image> EnrothTextureManager::GetOrCreateSpriteBitmap(const String &sprite_name) {
	const auto lod_sprite_id = _asset_loader.FindSprite(sprite_name.ascii().ptr());

	ERR_FAIL_COND_V_MSG(!lod_sprite_id, {}, "Sprite not found: " + sprite_name);

	const auto it = _sprite_bitmaps.find({ lod_sprite_id });
	if (it != _sprite_bitmaps.end())
		return it->value;

	const auto lod_sprite = _asset_loader.LoadSprite(sprite_name.ascii().ptr());

	ERR_FAIL_COND_V_MSG(!lod_sprite, {}, "Failed to load sprite: " + sprite_name);

	const auto bitmap = EnrothSpriteImporter::ImportBitmap(lod_sprite);

	ERR_FAIL_COND_V_MSG(bitmap.is_null(), {}, "Failed to create sprite bitmap: " + sprite_name);

	_sprite_bitmaps.insert({ lod_sprite_id }, bitmap);
	_sprite_headers.insert(lod_sprite_id, { lod_sprite });

	return bitmap;
}

Ref<Image> EnrothTextureManager::GetOrCreateSpriteImage(const String &sprite_name, uint16_t palette_id) {
	const auto lod_sprite_id = _asset_loader.FindSprite(sprite_name.ascii().ptr());

	ERR_FAIL_COND_V_MSG(!lod_sprite_id, {}, "Sprite not found: " + sprite_name);

	if (palette_id == InvalidPaletteId) {
		const auto it = _sprite_default_images.find({ lod_sprite_id });
		if (it != _sprite_default_images.end())
			return it->value;
	} else {
		const auto it = _images.find({ lod_sprite_id, palette_id });
		if (it != _images.end())
			return it->value;
	}

	const auto bitmap = GetOrCreateSpriteBitmap(sprite_name);

	ERR_FAIL_COND_V_MSG(bitmap.is_null(), {}, "Failed to get sprite bitmap: " + sprite_name);

	const auto sprite_header = _sprite_headers[lod_sprite_id];

	ERR_FAIL_COND_V_MSG(!CheckSpriteImage(sprite_header, bitmap), {}, "Sprite bitmap doesn't match header: " + sprite_name);

	const auto image = EnrothSpriteImporter(_asset_loader).Import(bitmap, palette_id == InvalidPaletteId ? sprite_header.palette_id : palette_id);

	ERR_FAIL_COND_V_MSG(image.is_null(), {}, "Failed to create sprite image: " + sprite_name);

	if (palette_id == InvalidPaletteId) {
		palette_id = sprite_header.palette_id;

		_sprite_default_images.insert({ lod_sprite_id }, image);
	}

	_images.insert({ lod_sprite_id, palette_id }, image);

	return image;
}

Ref<Texture2D> EnrothTextureManager::GetOrCreateSpriteTexture(const String &sprite_name, uint16_t palette_id) {
	const auto lod_sprite_id = _asset_loader.FindSprite(sprite_name.ascii().ptr());

	ERR_FAIL_COND_V_MSG(!lod_sprite_id, {}, "Sprite not found: " + sprite_name);

	if (palette_id == InvalidPaletteId) {
		const auto it = _sprite_default_textures.find({ lod_sprite_id });
		if (it != _sprite_default_textures.end())
			return it->value;
	} else {
		const auto it = _textures.find({ lod_sprite_id, palette_id });
		if (it != _textures.end())
			return it->value;
	}

	const auto image = GetOrCreateSpriteImage(sprite_name, palette_id);

	ERR_FAIL_COND_V_MSG(image.is_null(), {}, "Failed to get sprite image: " + sprite_name);

	const auto sprite_header = _sprite_headers[lod_sprite_id];

	ERR_FAIL_COND_V_MSG(!CheckSpriteImage(sprite_header, image), {}, "Sprite image doesn't match header: " + sprite_name);

	const auto texture = ImageTexture::create_from_image(image);

	ERR_FAIL_COND_V_MSG(texture.is_null(), {}, "Failed to create sprite texture: " + sprite_name);

	if (palette_id == InvalidPaletteId) {
		palette_id = sprite_header.palette_id;

		_sprite_default_textures.insert({ lod_sprite_id }, texture);
	}

	_textures.insert({ lod_sprite_id, palette_id }, texture);

	return texture;
}

String EnrothTextureManager::TranslateBitmapTextureName(const String &texture_name) {
	if (texture_name.begins_with("wtrdr"))
		return "h" + texture_name;
	return texture_name;
}

bool EnrothTextureManager::CheckSpriteImage(const SpriteHeader &header, const Ref<Image> &image) {
	return
		image->get_width() == header.width &&
		image->get_height() == header.height;
}
