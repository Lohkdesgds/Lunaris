#pragma once

#include <Lunaris/__macro/macros.h>

#include <memory>
#include <optional>
#include <stdexcept>

namespace Lunaris {

	/// <summary>
	/// <para>tie is a easy, shareable, referenceable tool.</para>
	/// <para>You can create it by itself or reference a memory somewhere (be sure that memory is there while this is used!).</para>
	/// <para>Self creation and referencing is 100% safe, but referencing existing variables may not. It depends on you. Because of that, you have to explicitly call ref() or tell the constructor to reference (no copy).</para>
	/// </summary>
	template<typename T>
	class tie {
		std::shared_ptr<T> _ptr; // new if self
	public:
		/// <summary>
		/// <para>Create as a simple variable.</para>
		/// </summary>
		/// <param name="{bool}">Create with new already? (useful for ref()ing it later, you can start null if false).</param>
		tie(const bool = true);

		/// <summary>
		/// <para>Reference a tie directly.</para>
		/// </summary>
		/// <param name="{tie}">Another tie.</param>
		tie(const tie&);

		/// <summary>
		/// <para>Move that tie to this.</para>
		/// </summary>
		/// <param name="{tie&amp;&amp;}">Another tie.</param>
		tie(tie&&);

		/// <summary>
		/// <para>Copy value to this. As constructor, this creates new variable internally.</para>
		/// </summary>
		/// <param name="{T}">A value to copy.</param>
		tie(const T&);

		/// <summary>
		/// <para>Move value to this. As constructor, this creates new variable internally.</para>
		/// </summary>
		/// <param name="{T&amp;&amp;}">A value to move.</param>
		tie(T&&);

		/// <summary>
		/// <para>Reference a tie directly.</para>
		/// </summary>
		/// <param name="{tie}">Another tie.</param>
		const tie& operator=(const tie&);

		/// <summary>
		/// <para>Move that tie to this.</para>
		/// </summary>
		/// <param name="{tie&amp;&amp;}">Another tie.</param>
		void operator=(tie&&);

		/// <summary>
		/// <para>Copy value to this.</para>
		/// <para>The referenced variable inside this gets the value.</para>
		/// </summary>
		/// <param name="{T}">A value to copy.</param>
		const T& operator=(const T&);

		/// <summary>
		/// <para>Move value to this.</para>
		/// <para>The referenced variable inside this gets the value.</para>
		/// </summary>
		/// <param name="{T&amp;&amp;}">A value to move.</param>
		void operator=(T&&);

		/// <summary>
		/// <para>Direct access to T.</para>
		/// </summary>
		/// <returns>{T*} variable referenced inside.</returns>
		const T* operator->() const;

		/// <summary>
		/// <para>Direct access to T.</para>
		/// </summary>
		/// <returns>{T*} variable referenced inside.</returns>
		T* operator->();

		/// <summary>
		/// <para>Direct access to T.</para>
		/// </summary>
		/// <returns>{T&amp;} variable referenced inside.</returns>
		const T& operator*() const;

		/// <summary>
		/// <para>Direct access to T.</para>
		/// </summary>
		/// <returns>{T&amp;} variable referenced inside.</returns>
		T& operator*();

		/// <summary>
		/// <para>Compare internal value to this.</para>
		/// </summary>
		/// <param name="{T}">Another value to compare.</param>
		/// <returns>{bool} True if equal.</returns>
		bool operator==(const T&) const;

		/// <summary>
		/// <para>Check if another tie is the same reference as this one.</para>
		/// </summary>
		/// <param name="{tie}">Check tie references.</param>
		/// <returns>{bool} True if they share the same reference.</returns>
		bool operator==(const tie&) const;

		/// <summary>
		/// <para>Compare internal value to this.</para>
		/// </summary>
		/// <param name="{T}">Another value to compare.</param>
		/// <returns>{bool} True if not equal.</returns>
		bool operator!=(const T&) const;

		/// <summary>
		/// <para>Check if another tie is the same reference as this one.</para>
		/// </summary>
		/// <param name="{tie}">Check tie references.</param>
		/// <returns>{bool} True if they DON'T share the same reference.</returns>
		bool operator!=(const tie&) const;

		/// <summary>
		/// <para>Unrefs any referenced memory and create new internally (so it's valid for operators and so on).</para>
		/// </summary>
		/// <returns>{std::optional&lt;T&gt;} The variable, moved, or empty if previously null.</returns>
		std::optional<T> unref();

		/// <summary>
		/// <para>Reference a variable instead (safety depends on you).</para>
		/// </summary>
		/// <param name="{T&amp;}">A variable to reference.</param>
		/// <returns>{std::optional&lt;T&gt;} The variable, moved, or empty if previously null.</returns>
		std::optional<T> ref(T&);

		/// <summary>
		/// <para>Like move, cut reference from that tie to this.</para>
		/// </summary>
		/// <param name="{tie&amp;&amp;}">Another tie</param>
		void swap(tie&&);

		/// <summary>
		/// <para>Is it valid (not null?).</para>
		/// </summary>
		/// <returns>{bool} True if has valid pointer (not null).</returns>
		bool valid() const;

		/// <summary>
		/// <para>Is it invalid (null?).</para>
		/// </summary>
		/// <returns>{bool} True if null.</returns>
		bool empty() const;
	};
}

#include "tie.ipp"