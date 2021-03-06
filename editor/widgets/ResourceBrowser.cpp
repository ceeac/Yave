/*******************************
Copyright (c) 2016-2020 Grégoire Angerand

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**********************************/

#include "ResourceBrowser.h"
#include "SceneImporter.h"
#include "ImageImporter.h"

#include <editor/context/EditorContext.h>
#include <editor/utils/assets.h>
#include <editor/utils/ui.h>

#include <y/io2/Buffer.h>

#include <imgui/yave_imgui.h>


namespace editor {

ResourceBrowser::ResourceBrowser(ContextPtr ctx) : ResourceBrowser(ctx, ICON_FA_FOLDER_OPEN " Resource Browser") {
}

ResourceBrowser::ResourceBrowser(ContextPtr ctx, std::string_view title) :
		FileSystemView(ctx->asset_store().filesystem(), title),
		ContextLinked(ctx) {

	set_flags(ImGuiWindowFlags_NoScrollbar);
	path_changed();
}

AssetId ResourceBrowser::asset_id(std::string_view name) const {
	return context()->asset_store().id(name).unwrap_or(AssetId());
}

AssetType ResourceBrowser::read_file_type(AssetId id) const {
	return context()->asset_store().asset_type(id).unwrap_or(AssetType::Unknown);
}

bool ResourceBrowser::is_searching() const {
	return !!_search_results;
}



void ResourceBrowser::paint_import_menu() {
	if(ImGui::Selectable("Import objects")) {
		context()->ui().add<SceneImporter>(context(), path());
	}
	if(ImGui::Selectable("Import image")) {
		context()->ui().add<ImageImporter>(context(), path());
	}
}

void ResourceBrowser::paint_context_menu() {
	FileSystemView::paint_context_menu();

	ImGui::Separator();

	paint_import_menu();

	ImGui::Separator();

	if(ImGui::Selectable("Create material")) {
		const SimpleMaterialData material;
		io2::Buffer buffer;
		serde3::WritableArchive arc(buffer);
		if(arc.serialize(material)) {
			buffer.reset();
			AssetStore& store = context()->asset_store();
			if(!store.import(buffer, store.filesystem()->join(path(), "new material"), AssetType::Material)) {
				log_msg("Unable to import new material.", Log::Error);
			}
		} else {
			log_msg("Unable to create new material.", Log::Error);
		}

		refresh_all();
	}
}

void ResourceBrowser::paint_preview(float width) {
	if(_preview_id != AssetId::invalid_id()) {
		const auto thumb_data = context()->thumbmail_cache().get_thumbmail(_preview_id);
		auto paint_properties = [&] {
			ImGui::Text("ID: 0x%.8x", unsigned(_preview_id.id()));
			for(const auto& [name, value] : thumb_data.properties) {
				ImGui::TextUnformatted(fmt_c_str("%: %", name, value));
			}
		};

		if(width > 0.0f) {
			ImGui::SameLine();
			ImGui::BeginGroup();
			if(TextureView* image = thumb_data.image) {
				ImGui::Image(image, math::Vec2(width));
			}
			paint_properties();
			ImGui::EndGroup();
		}

		{
			ImGui::BeginTooltip();
			paint_properties();
			ImGui::EndTooltip();
		}
	}
}

void ResourceBrowser::paint_path_bar() {
	ImGui::PushID("##pathbar");

	if(ImGui::Button(ICON_FA_PLUS " Import")) {
		ImGui::OpenPopup("##importmenu");
	}

	if(ImGui::BeginPopup("##importmenu")) {
		paint_import_menu();
		ImGui::EndPopup();
	}

	{
		ImGui::PushStyleColor(ImGuiCol_Button, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);

		const usize path_size = _path_pieces.size();
		for(usize i = 0; i != path_size; ++i) {
			const usize index = path_size - 1 - i;
			const auto& piece = _path_pieces[index];

			auto full_path = [&] {
				core::String p = piece;
				for(usize j = index + 1; j < path_size; ++j) {
					p = filesystem()->join(_path_pieces[j], p);
				}
				return p;
			};

			ImGui::SameLine();
			if(ImGui::Button(fmt_c_str("%##%", piece.is_empty() ? "/" : piece, i))) {
				_set_path_deferred = full_path();
			}

			ImGui::SameLine();
			const core::String menu_name = fmt("##jumpmenu_%", i);
			if(ImGui::Button(fmt_c_str(ICON_FA_ANGLE_RIGHT "##%", i))) {
				_jump_menu.make_empty();
				filesystem()->for_each(full_path(), [this](std::string_view f) {
						if(filesystem()->is_directory(f).unwrap_or(false)) {
							_jump_menu << f;
						}
					}).ignore();

				ImGui::OpenPopup(menu_name.data());
			}

			if(ImGui::BeginPopup(menu_name.data())) {
				for(const core::String& dir : _jump_menu) {
					if(ImGui::Selectable(fmt_c_str(ICON_FA_FOLDER " %", dir))) {
						_set_path_deferred = filesystem()->join(full_path(), dir);
					}
				}
				if(_jump_menu.is_empty()) {
					ImGui::Selectable("no subdirectories", false, ImGuiSelectableFlags_Disabled);
				}
				ImGui::EndPopup();
			}
		}

		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
	}

	{
		bool has_seach_bar = false;
		if(dynamic_cast<const SearchableFileSystemModel*>(filesystem())) {
			const usize search_bar_size = 200;
			if(search_bar_size < size().x()) {
				has_seach_bar = true;
				ImGui::SameLine(size().x() - (search_bar_size + 24));
				ImGui::SetNextItemWidth(search_bar_size);
				/*if(ImGui::InputTextWithHint("##searchbar", " " ICON_FA_SEARCH, _search_pattern.data(), _search_pattern.size())) {
					update_search();
				}*/
				if(ImGui::InputText(ICON_FA_SEARCH, _search_pattern.data(), _search_pattern.size())) {
					update_search();
				}
			}
		}

		if(!has_seach_bar || !_search_pattern[0]) {
			_search_results = nullptr;
		}
	}

	ImGui::PopID();
}

void ResourceBrowser::paint_search_results(float width) {
	Y_TODO(Replace by table when available)

	_preview_id = AssetId::invalid_id();
	ImGui::BeginChild("##searchresults", ImVec2(width, 0), true);
	{
		imgui::alternating_rows_background();
		for(const Entry& entry : *_search_results) {
			if(ImGui::Selectable(fmt_c_str("% %", entry.icon, entry.name))) {
				if(const AssetId id = asset_id(entry.name); id != AssetId::invalid_id()) {
					asset_selected(id);
				}
				_set_path_deferred = entry.name;
			}

			if(ImGui::IsItemHovered()) {
				if(const AssetId id = asset_id(entry.name); id != AssetId::invalid_id()) {
					_preview_id = id;
				}
			}
		}

		if(_search_results->is_empty()) {
			ImGui::Selectable("No results", false, ImGuiSelectableFlags_Disabled);
		}
	}
	ImGui::EndChild();
}



void ResourceBrowser::paint_ui(CmdBufferRecorder& recorder, const FrameToken& token) {
	unused(recorder, token);
	y_profile();


	const float width = ImGui::GetWindowContentRegionWidth();
	const float tree_width = 0.0f;
	const bool render_preview = width - tree_width > 400.0f;
	const float list_width = render_preview ? width - tree_width - 200.0f : 0.0f;
	const float preview_width = width - tree_width - list_width;

	{
		paint_path_bar();
	}

	if(is_searching()) {
		paint_search_results(list_width);
	} else {
		ImGui::BeginChild("##assets", ImVec2(list_width, 0.0f));
		FileSystemView::paint_ui(recorder, token);
		ImGui::EndChild();
	}

	paint_preview(render_preview ? preview_width : 0.0f);

	if(_set_path_deferred != path()) {
		set_path(_set_path_deferred);
		_search_results = nullptr;
	}
}






void ResourceBrowser::path_changed() {
	_set_path_deferred = path();
	_path_pieces.make_empty();

	core::String full_path = path();
	while(!full_path.is_empty()) {
		if(const auto parent = filesystem()->parent_path(full_path)) {
			const usize parent_size = parent.unwrap().size();
			if(parent_size < full_path.size()) {
				const bool has_separator = full_path[parent_size] == '/';
				_path_pieces << full_path.sub_str(parent_size + has_separator);
				full_path = parent.unwrap();
				continue;
			}
		}
		break;
	}

	_path_pieces << ""; // Root
}



void ResourceBrowser::update() {
	if(is_searching()) {
		update_search();
	}
	FileSystemView::update();
}

void ResourceBrowser::update_search() {
	_search_results = nullptr;

	if(const auto* searchable = dynamic_cast<const SearchableFileSystemModel*>(filesystem())) {
		std::string_view pattern = std::string_view(_search_pattern.data());
		const core::String full_pattern = fmt("%%%%", path(), "%", pattern, "%");

		if(auto result = searchable->search(full_pattern)) {
			_search_results = std::make_unique<core::Vector<Entry>>();

			for(const core::String& full_name : result.unwrap()) {
				const core::String name = searchable->filename(full_name);
				const bool is_dir = filesystem()->is_directory(full_name).unwrap_or(false);
				const EntryType type = is_dir ? EntryType::Directory : EntryType::File;
				if(auto icon = entry_icon(full_name, type)) {
					_search_results->emplace_back(Entry{full_name, type, std::move(icon.unwrap())});
				}
			}
		} else {
			log_msg("Search failed.", Log::Warning);
		}
	}
}

core::Result<core::String> ResourceBrowser::entry_icon(const core::String& full_name, EntryType type) const {
	if(type == EntryType::Directory) {
		return FileSystemView::entry_icon(full_name, type);
	}
	if(const AssetId id = asset_id(full_name); id != AssetId::invalid_id()) {
		const AssetType asset = read_file_type(id);
		return core::Ok(core::String(asset_type_icon(asset)));
	}
	return core::Err();
}

void ResourceBrowser::entry_hoverred(const Entry* entry) {
	_preview_id = AssetId::invalid_id();
	if(entry) {
		if(const AssetId id = asset_id(entry_full_name(*entry)); id != AssetId::invalid_id()) {
			_preview_id = id;
		}
	}
}

void ResourceBrowser::entry_clicked(const Entry& entry) {
	const core::String full_name = entry_full_name(entry);
	if(const AssetId id = asset_id(full_name); id != AssetId::invalid_id()) {
		asset_selected(id);
	}
	FileSystemView::entry_clicked(entry);
}

}
