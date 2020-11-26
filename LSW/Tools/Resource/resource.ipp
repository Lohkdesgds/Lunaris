#pragma once

namespace LSW {
	namespace v5 {
		namespace Tools {

			template<typename T>
			inline Resource<T>::Resource(const Resource& a)
			{
				*this = a;
			}

			template<typename T>
			inline Resource<T>::Resource(const std::shared_ptr<T>& a)
			{
				*this = a;
			}

			template<typename T>
			inline Resource<T>::Resource(Resource&& a) noexcept
			{
				*this = std::move(a);
			}

			template<typename T>
			inline Resource<T>::Resource(std::shared_ptr<T>&& a) noexcept
			{
				*this = std::move(a);
			}

			template<typename T>
			inline void Resource<T>::operator=(const Resource& a)
			{
				r = a.r;
			}

			template<typename T>
			inline void Resource<T>::operator=(const std::shared_ptr<T>& a)
			{
				r = a;
			}

			template<typename T>
			inline void Resource<T>::operator=(Resource&& a) noexcept
			{
				r = std::move(a.r);
			}

			template<typename T>
			inline void Resource<T>::operator=(std::shared_ptr<T>&& a) noexcept
			{
				r = std::move(a);
			}

			template<typename T>
			inline bool Resource<T>::empty() const
			{
				return r && r.get();
			}

			template<typename T>
			inline T* Resource<T>::operator->()
			{
				return r.get();
			}

			template<typename T>
			inline T* Resource<T>::operator->() const
			{
				return r.get();
			}

			template<typename T>
			inline T& Resource<T>::operator*()
			{
				return *r;
			}

			template<typename T>
			inline T& Resource<T>::operator*() const
			{
				return *r;
			}

			template<typename T>
			inline bool Resource<T>::operator!() const
			{
				return !empty();
			}

			template<typename T>
			inline bool Resource<T>::operator==(const Resource& o) const
			{
				return r == o.r;
			}

			template<typename T>
			inline Resource<T>::operator const T* () const
			{
				return r.get();
			}

			template<typename T>
			inline Resource<T>::operator T* ()
			{
				return r.get();
			}

			template<typename T>
			inline Resource<T>::operator std::shared_ptr<T>()
			{
				return r;
			}

			template<typename T>
			inline Resource<T>::operator const T& () const
			{
				return *r.get();
			}

			template<typename T>
			inline Resource<T>::operator T& ()
			{
				return *r.get();
			}

			template<typename T>
			inline T* Resource<T>::get()
			{
				return r.get();
			}

			template<typename T>
			inline const T* Resource<T>::get() const
			{
				return r.get();
			}

			template<typename T>
			inline void Resource<T>::swap(Resource& o)
			{
				r.swap(o.r);
			}

			template<typename T>
			inline std::shared_ptr<T>& Resource<T>::get_shared()
			{
				return r;
			}

		}
	}
}
