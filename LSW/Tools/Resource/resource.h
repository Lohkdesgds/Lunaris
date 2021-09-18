#pragma once

#include <string>
#include <functional>
#include <vector>
#include <mutex>

#include "../../Handling/Abort/abort.h"
#include "../SuperMutex/supermutex.h"


namespace LSW {
	namespace v5 {
		namespace Tools {

			/// <summary>
			/// <para>Resource is a shared_ptr, but smarter when using automatic casting and some extras.</para>
			/// <para>You can go back and forth from std::shared_ptr to this.</para>
			/// </summary>
			template<typename T>
			class Resource {
				std::shared_ptr<T> r;
			public:
				Resource() = default;

				/// <summary>
				/// <para>Reference constructor.</para>
				/// <para>This will reference the same variable.</para>
				/// </summary>
				/// <param name="{Resource}">A Resource.</param>
				Resource(const Resource&);

				/// <summary>
				/// <para>Reference constructor.</para>
				/// <para>This will reference the same variable.</para>
				/// </summary>
				/// <param name="{std::shared_ptr}">A smart pointer.</param>
				Resource(const std::shared_ptr<T>&);

				/// <summary>
				/// <para>Move constructor.</para>
				/// </summary>
				/// <param name="{Resource}">The Resource that will have data moved.</param>
				Resource(Resource&&) noexcept;

				/// <summary>
				/// <para>Move constructor.</para>
				/// </summary>
				/// <param name="{std::shared_ptr}">The smart pointer that will have data moved.</param>
				Resource(std::shared_ptr<T>&&) noexcept;

				/// <summary>
				/// <para>Reference operator.</para>
				/// <para>This will reference the same variable.</para>
				/// </summary>
				/// <param name="{Resource}">A Resource.</param>
				void operator=(const Resource&);

				/// <summary>
				/// <para>Reference operator.</para>
				/// <para>This will reference the same variable.</para>
				/// </summary>
				/// <param name="{std::shared_ptr}">A smart pointer.</param>
				void operator=(const std::shared_ptr<T>&);

				/// <summary>
				/// <para>Move operator.</para>
				/// </summary>
				/// <param name="{Resource}">The Resource that will have data moved.</param>
				void operator=(Resource&&) noexcept;

				/// <summary>
				/// <para>Move operator.</para>
				/// </summary>
				/// <param name="{std::shared_ptr}">The smart pointer that will have data moved.</param>
				void operator=(std::shared_ptr<T>&&) noexcept;

				/// <summary>
				/// <para>Check if internal data is present.</para>
				/// </summary>
				/// <returns>{bool} True if empty.</returns>
				bool empty() const;

				/// <summary>
				/// <para>Direct access to internal data.</para>
				/// </summary>
				/// <returns>{T*} Itself, if present.</returns>
				T* operator->();

				/// <summary>
				/// <para>Direct access to internal data.</para>
				/// </summary>
				/// <returns>{T*} Itself, if present.</returns>
				T* operator->() const;

				/// <summary>
				/// <para>Direct access to internal data.</para>
				/// </summary>
				/// <returns>{T} Itself, if present.</returns>
				T& operator*();

				/// <summary>
				/// <para>Direct access to internal data.</para>
				/// </summary>
				/// <returns>{T} Itself, if present.</returns>
				T& operator*() const;

				/// <summary>
				/// <para>Same as empty.</para>
				/// </summary>
				/// <returns>{bool} True if empty.</returns>
				bool operator!() const;

				/// <summary>
				/// <para>Compare two resources to check if they have the same pointer.</para>
				/// </summary>
				/// <param name="{Resource}">Another Resource.</param>
				/// <returns>{bool} True if same.</returns>
				bool operator==(const Resource&) const;

				/// <summary>
				/// <para>Direct cast to shared pointer.</para>
				/// </summary>
				operator std::shared_ptr<T>();

				/// <summary>
				/// <para>Direct cast to T*.</para>
				/// </summary>
				operator const T* () const;

				/// <summary>
				/// <para>Direct cast to T*.</para>
				/// </summary>
				operator T* ();

				/// <summary>
				/// <para>Direct cast to T.</para>
				/// </summary>
				operator const T& () const;

				/// <summary>
				/// <para>Direct cast to T.</para>
				/// </summary>
				operator T& ();

				/// <summary>
				/// <para>Get internal data.</para>
				/// </summary>
				/// <returns>{T*} The data.</returns>
				T* get();

				/// <summary>
				/// <para>Get internal data.</para>
				/// </summary>
				/// <returns>{T*} The data.</returns>
				const T* get() const;

				/// <summary>
				/// <para>Swap Resouces.</para>
				/// </summary>
				/// <param name="{Resource}">The other Resource.</param>
				void swap(Resource&);

				/// <summary>
				/// <para>Gets the internal shared pointer.</para>
				/// </summary>
				/// <returns>{std::shared_ptr} Internal shared pointer.</returns>
				std::shared_ptr<T>& get_shared();

				/// <summary>
				/// <para>Reset to invalid pointer.</para>
				/// </summary>
				void reset();
			};

		}
	}
}

#include "resource.ipp"