#pragma once

namespace LSW {
	namespace v5 {
		namespace Tools {

			template<typename T>
			template<typename Q, std::enable_if_t<is_type_complete_v<Q>, int>>
			inline std::unique_ptr<T, std::function<void(T*)>>* AdvancedShared<T>::gen_unique(T* v, std::function<void(T*)> f)
			{
				if (f) {
					auto* gen = new std::unique_ptr<T, std::function<void(T*)>>(v, f);
					return std::move(gen);
				}
				else {
					auto* gen = new std::unique_ptr<T, std::function<void(T*)>>(v, [](T* t) { delete t; });
					return std::move(gen);
				}
			}

			template<typename T>
			template<typename Q, std::enable_if_t<!is_type_complete_v<Q>, int>>
			inline std::unique_ptr<T, std::function<void(T*)>>* AdvancedShared<T>::gen_unique(T* v, std::function<void(T*)> f)
			{
				if (f) {
					auto* gen = new std::unique_ptr<T, std::function<void(T*)>>(v, f);
					return std::move(gen);
				}
				else {
					throw Handling::Abort(__FUNCSIG__, "Incomplete type cannot be created like this! Function invalid when creating a AdvancedShared.");
				}
			}

			template<typename T>
			template<typename Q, std::enable_if_t<is_type_complete_v<Q>, int>>
			inline AdvancedShared<T>::AdvancedShared(const bool creat)
			{
				if (creat) create();
			}

			template<typename T>
			template<typename Q, std::enable_if_t<!is_type_complete_v<Q>, int>>
			inline AdvancedShared<T>::AdvancedShared()
			{
				// nothing to do.
			}

			template<typename T>
			template<typename Q, std::enable_if_t<is_type_complete_v<Q>, int>>
			inline AdvancedShared<T>::AdvancedShared(const T& v)
			{
				create(v);
			}

			template<typename T>
			inline AdvancedShared<T>::AdvancedShared(T* p, std::function<void(T*)> f)
			{
				if (f) create(p, f);
				else create(p);
			}

			template<typename T>
			inline AdvancedShared<T>::AdvancedShared(const AdvancedShared& c)
			{
				*this = c;
			}

			template<typename T>
			inline AdvancedShared<T>::AdvancedShared(AdvancedShared&& m)
			{
				*this = std::move(m);
			}

			template<typename T>
			inline void AdvancedShared<T>::operator=(const AdvancedShared& c)
			{
				data = c.data;
			}

			template<typename T>
			inline void AdvancedShared<T>::operator=(AdvancedShared&& m)
			{
				data = std::move(m.data);
			}

			template<typename T>
			template<typename Q, std::enable_if_t<is_type_complete_v<Q>, int>>
			inline void AdvancedShared<T>::create()
			{

				data = std::shared_ptr<std::unique_ptr<T, std::function<void(T*)>>>(gen_unique(new T()));
			}

			template<typename T>
			template<typename Q, std::enable_if_t<is_type_complete_v<Q>, int>>
			inline void AdvancedShared<T>::create(const T& val)
			{
				create();
				**this = val;
			}

			template<typename T>
			inline void AdvancedShared<T>::create(T* pre)
			{
				if (!pre) throw Handling::Abort(__FUNCSIG__, "Invalid pointer.");
				data = std::shared_ptr<std::unique_ptr<T, std::function<void(T*)>>>(gen_unique(pre));
			}

			template<typename T>
			inline void AdvancedShared<T>::create(T* pre, std::function<void(T*)> f)
			{
				if (!pre || !f) throw Handling::Abort(__FUNCSIG__, "Invalid pointer or function.");
				data = std::shared_ptr<std::unique_ptr<T, std::function<void(T*)>>>(gen_unique(pre, f));
			}

			template<typename T>
			inline void AdvancedShared<T>::swap(T*& oth)
			{
				if (data) {
					T* b4 = data->get();
					data->reset(oth);
					oth = b4;
				}
				else {
					create(oth);
					oth = nullptr;
				}
			}

			template<typename T>
			inline void AdvancedShared<T>::swap_destroy(T* oth)
			{
				auto del = data->get_deleter();

				if (data) {
					T* b4 = data->get();
					data->reset(oth);
					del(b4);
				}
				else {
					create(oth);
					oth = nullptr;
				}
			}

			template<typename T>
			inline void AdvancedShared<T>::swap(AdvancedShared& s)
			{
				data.swap(s.data);
			}

			template<typename T>
			inline void AdvancedShared<T>::reset_this()
			{
				data.reset();
			}

			template<typename T>
			inline void AdvancedShared<T>::reset_all()
			{
				if (data) data->reset();
			}

			template<typename T>
			inline AdvancedShared<T>::operator T* ()
			{
				if (!data) throw Handling::Abort(__FUNCSIG__, "Null data.");
				return data->get();
			}

			template<typename T>
			inline AdvancedShared<T>::operator const T* () const
			{
				if (!data) throw Handling::Abort(__FUNCSIG__, "Null data.");
				return data->get();
			}

			template<typename T>
			inline T& AdvancedShared<T>::operator*()
			{
				if (!data || !*data) throw Handling::Abort(__FUNCSIG__, "Null data.");
				return *data->get();
			}

			template<typename T>
			inline const T& AdvancedShared<T>::operator*() const
			{
				if (!data || !*data) throw Handling::Abort(__FUNCSIG__, "Null data.");
				return *data->get();
			}

			template<typename T>
			inline AdvancedShared<T>::operator bool() const
			{
				return !null();
			}

			template<typename T>
			inline bool AdvancedShared<T>::operator!() const
			{
				return null();
			}

			template<typename T>
			inline bool AdvancedShared<T>::null() const
			{
				return (data.get() == nullptr || data->get() == nullptr);
			}

			template<typename T>
			inline size_t AdvancedShared<T>::use_count() const
			{
				return data.use_count();
			}

			template<typename T>
			inline T* AdvancedShared<T>::get()
			{
				if (!data || !*data) throw Handling::Abort(__FUNCSIG__, "Null data.");
				return data->get();
			}

			template<typename T>
			inline T* AdvancedShared<T>::get() const
			{
				if (!data || !*data) throw Handling::Abort(__FUNCSIG__, "Null data.");
				return data->get();
			}

			template<typename T>
			inline const std::shared_ptr<std::unique_ptr<T, std::function<void(T*)>>>& AdvancedShared<T>::get_internal_data() const
			{
				return data;
			}

		}
	}
}