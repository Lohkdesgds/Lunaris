#pragma once

#include "../../Handling/Abort/abort.h"
#include "../../Handling/Initialize/initialize.h"
#include "../../Tools/FastFunction/fastfunction.h"
#include "../Bitmap/bitmap.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			/// <summary>
			/// <para>Target manages the target Bitmap based on index. When you create a Display, you set a index for it, then you can have multiple configurations by using unique indexes!</para>
			/// </summary>
			class Target {
				static std::unordered_map<size_t, Tools::FastFunction<Bitmap>> func;
				const size_t i;
			public:
				/// <summary>
				/// <para>Create a Target targeting a index.</para>
				/// </summary>
				/// <param name="{size_t}">Index.</param>
				Target(const size_t);

				/// <summary>
				/// <para>Overwrite target Bitmap in this index.</para>
				/// </summary>
				/// <param name="{Bitmap}">New target.</param>
				void set(const Bitmap&);

				/// <summary>
				/// <para>Overwrite target Bitmap in this index with a function.</para>
				/// </summary>
				/// <param name="{std::function}">New dynamic target.</param>
				void set(const std::function<Bitmap(void)>&);

				/// <summary>
				/// <para>Apply target Bitmap as target in this thread.</para>
				/// </summary>
				void apply();
				
				/// <summary>
				/// <para>Get a reference to the target.</para>
				/// </summary>
				/// <returns>{Bitmap} A reference to target.</returns>
				Bitmap get();
			};			

		}
	}
}