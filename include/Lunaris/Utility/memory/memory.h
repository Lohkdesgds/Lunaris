#pragma once

#include <Lunaris/__macro/macros.h>

#include <memory>
#include <stdexcept>

namespace Lunaris {

	template<typename T>
	class hybrid_memory {
		std::shared_ptr<std::unique_ptr<T>> m_ptr;
	public:
		hybrid_memory() = default;

		hybrid_memory(const hybrid_memory&);
		hybrid_memory(hybrid_memory&&) noexcept;
		void operator=(const hybrid_memory&);
		void operator=(hybrid_memory&&) noexcept;

		/// <summary>
		/// <para>Compare if both share same unique_ptr</para>
		/// </summary>
		/// <param name="{hybrid_memory}">Another hybrid_memory.</param>
		/// <returns>{bool} True if they are copies of each other (reference same memory).</returns>
		bool operator==(const hybrid_memory&) const;

		/// <summary>
		/// <para>Compare if both don't share same unique_ptr</para>
		/// </summary>
		/// <param name="{hybrid_memory}">Another hybrid_memory.</param>
		/// <returns>{bool} True if they are NOT copies of each other (reference same memory).</returns>
		bool operator!=(const hybrid_memory&) const;

		/// <summary>
		/// <para>Get pointer stored and handled inside this object (don't ever delete this).</para>
		/// </summary>
		/// <returns>{T*} Pointer to stored value.</returns>
		const T* get() const;

		/// <summary>
		/// <para>Get pointer stored and handled inside this object (don't ever delete this).</para>
		/// </summary>
		/// <returns>{T*} Pointer to stored value.</returns>
		T* get();

		/// <summary>
		/// <para>Use this as direct pointer to variable inside.</para>
		/// </summary>
		/// <returns>{T*} The variable pointer itself.</returns>
		const T* operator->() const;

		/// <summary>
		/// <para>Use this as direct pointer to variable inside.</para>
		/// </summary>
		/// <returns>{T*} The variable pointer itself.</returns>
		T* operator->();

		/// <summary>
		/// <para>Use this to get direct reference to variable internally (assuming it's not null).</para>
		/// </summary>
		/// <returns>{T&amp;} The variable reference itself.</returns>
		const T& operator*() const;

		/// <summary>
		/// <para>Use this to get direct reference to variable internally (assuming it's not null).</para>
		/// </summary>
		/// <returns>{T&amp;} The variable reference itself.</returns>
		T& operator*();

		/// <summary>
		/// <para>If the pointer inside is valid (false means nullptr).</para>
		/// </summary>
		/// <returns>{bool} Is not null?</returns>
		bool valid() const;

		/// <summary>
		/// <para>If the pointer inside is nullptr.</para>
		/// </summary>
		/// <returns>{bool} Is null?</returns>
		bool empty() const;

		/// <summary>
		/// <para>Reset this object reference (if someone else is pointing to the object, it is not destroyed).</para>
		/// <para>This only affects this object.</para>
		/// </summary>
		void reset_this();

		/// <summary>
		/// <para>Reset all reference's value to null.</para>
		/// <para>This affects EVERY OBJECT that are pointing to same data.</para>
		/// </summary>
		/// <returns>{unique_ptr} The value that was shared (moved).</returns>
		std::unique_ptr<T> reset_shared();

		/// <summary>
		/// <para>Replaces current reference to a new one with this value.</para>
		/// <para>This only affects this object.</para>
		/// </summary>
		/// <param name="{T&amp;&amp;}">Value to be set.</param>
		void replace_this(T&&);

		/// <summary>
		/// <para>Replaces all reference's shared value with this.</para>
		/// <para>This affects EVERY OBJECT that are pointing to same data.</para>
		/// </summary>
		/// <param name="{T&amp;&amp;}">Value to be set.</param>
		void replace_shared(T&&);

		/// <summary>
		/// <para>Replaces current reference to a new one with this smart pointer.</para>
		/// <para>This only affects this object.</para>
		/// </summary>
		/// <param name="{unique_ptr}">Value to be set.</param>
		void replace_this(std::unique_ptr<T>&&);

		/// <summary>
		/// <para>Replaces all reference's shared value with this smart pointer.</para>
		/// <para>This affects EVERY OBJECT that are pointing to same data.</para>
		/// </summary>
		/// <param name="{unique_ptr}">Value to be set.</param>
		void replace_shared(std::unique_ptr<T>&&);

		/// <summary>
		/// <para>Makes this follow that other hybrid memory.</para>
		/// <para>This only affects this object.</para>
		/// </summary>
		/// <param name="hybrid_memory">Memory to reference.</param>
		void replace_this(const hybrid_memory&);

		/// <summary>
		/// <para>Use count of the same data as this one.</para>
		/// </summary>
		/// <returns>{size_t} Amount of hybrid_memory referencing this same data.</returns>
		size_t use_count() const;
	};

	template<typename T>
	hybrid_memory<T> make_hybrid();
	template<typename T, typename K>
	hybrid_memory<T> make_hybrid_derived();

	template<typename T, class... Args>
	hybrid_memory<T> make_hybrid(Args&&...);
	template<typename T, typename K, class... Args>
	hybrid_memory<T> make_hybrid_derived(Args&&...);
}

#include "memory.ipp"