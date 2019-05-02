/*******************************
Copyright (c) 2016-2019 Gr�goire Angerand

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
#ifndef EDITOR_CONTEXT_SELECTION_H
#define EDITOR_CONTEXT_SELECTION_H

#include <editor/editor.h>
#include <yave/objects/Light.h>
#include <yave/objects/Transformable.h>

#include <yave/material/Material.h>

namespace editor {

class Selection {
	public:
		void flush_reload() {
			_material.flush_reload();
		}

		void set_selected(Light* sel) {
			_transformable = sel;
			_light = sel;
		}

		void set_selected(Transformable* sel) {
			_transformable = sel;
			_light = nullptr;
		}

		void set_selected(std::nullptr_t) {
			_transformable = nullptr;
			_light = nullptr;
		}

		Transformable* selected() const {
			return _transformable;
		}

		Light* light() const {
			return _light;
		}


		void set_selected(const AssetPtr<Material>& sel) {
			_material = sel;
		}

		const auto& material() const {
			return _material;
		}

	private:
		NotOwner<Transformable*> _transformable = nullptr;
		NotOwner<Light*> _light = nullptr;
		AssetPtr<Material> _material;
};

}

#endif // EDITOR_CONTEXT_SELECTION_H
