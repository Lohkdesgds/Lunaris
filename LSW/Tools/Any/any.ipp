#pragma once

namespace LSW {
	namespace v5 {
		namespace Tools {
			
			template<typename... Args>
			inline Any::Any(Args... args) : std::any(args...)
			{
				// nothing, yes
			}

			template<typename T>
			inline T Any::get()
			{
				return std::any_cast<T>(*this);
			}
			
			template<typename T, std::enable_if_t<!std::is_same_v<r_cast_t<T>, char*>, int>>
			inline T Any::get() const
			{
				return std::any_cast<T>(*this);
			}

		}
	}
}