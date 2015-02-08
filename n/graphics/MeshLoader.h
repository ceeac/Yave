/*******************************
Copyright (C) 2013-2015 gregoire ANGERAND

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************/

#ifndef N_GRAPHICS_MESHLOADER
#define N_GRAPHICS_MESHLOADER

#include "MeshInstance.h"
#include <n/assets/Asset.h>
#include <n/assets/AssetBuffer.h>

namespace n {
namespace graphics {

class MeshLoader
{
	static MeshLoader *loader;
	public:
		template<typename T, typename... Args>
		class MeshDecoder
		{
			struct Runner
			{
				Runner() {
					getLoader()->addDec<Args...>(T());
				}
			};

			public:
				MeshDecoder() {
					n::unused(runner);
				}

				virtual internal::MeshInstance<> *operator()(Args...) = 0;

			private:
				static Runner runner;
		};

		template<typename... Args>
		static MeshInstance<> load(Args... args, bool async = true)  {
			MeshLoader *ld = getLoader();
			return async ? MeshInstance<>(ld->asyncBuffer.load(args...)) : MeshInstance<>(ld->immediateBuffer.load(args...));
		}

		template<typename... Args, typename T>
		void addDec(const T &t) {
			asyncBuffer.addLoader<Args...>(t);
			immediateBuffer.addLoader<Args...>(t);
		}

	private:
		static MeshLoader *getLoader() {
			if(!loader) {
				loader = new MeshLoader();
			}
			return loader;
		}

		MeshLoader() {
		}

		assets::AssetBuffer<internal::MeshInstance<>, assets::AsyncLoadingPolicy<internal::MeshInstance<>>> asyncBuffer;
		assets::AssetBuffer<internal::MeshInstance<>, assets::ImmediateLoadingPolicy<internal::MeshInstance<>>> immediateBuffer;

};

template<typename T, typename... Args>
typename MeshLoader::MeshDecoder<T, Args...>::Runner MeshLoader::MeshDecoder<T, Args...>::runner = MeshLoader::MeshDecoder<T, Args...>::Runner();


}
}


#endif // N_GRAPHICS_MESHLOADER

