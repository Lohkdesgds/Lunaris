#pragma once

namespace LSW {
	namespace v5 {
		namespace Tools {

			template<typename T>
			inline Resource<T> SuperResource<T>::get_main()
			{
				Tools::AutoLock luck(saves.m);
				return saves.main_o;
			}

			template<typename T>
			inline bool SuperResource<T>::set_main(const std::string id)
			{
				AutoLock luck(saves.m);
				for (auto& i : saves.objs) {
					if (i._id == id) {
						saves.main_o = i._data;
						return true;
					}
				}
				return false;
			}

			template<typename T>
			inline void SuperResource<T>::lock() 
			{
				saves.m.lock();
			}

			template<typename T>
			inline void SuperResource<T>::unlock()
			{
				saves.m.unlock();
			}

			template<typename T>
			inline auto SuperResource<T>::begin()
			{
				return saves.objs.begin();
			}

			template<typename T>
			inline auto SuperResource<T>::end()
			{
				return saves.objs.end();
			}

			template<typename T>
			inline bool SuperResource<T>::rename(const std::string id, const std::string new_id)
			{
				AutoLock luck(saves.m);
				for (auto& i : saves.objs) {
					if (i._id == id) {
						i._id = new_id;
						return true;
					}
				}
				return false;
			}

			template<typename T>
			inline bool SuperResource<T>::set_enabled(const std::string id, const bool enable)
			{
				AutoLock luck(saves.m);
				for (auto& i : saves.objs) {
					if (i._id == id) {
						i._enabled = enable;
						return true;
					}
				}
				return false;
			}

			template<typename T>
			inline size_t SuperResource<T>::rename(const std::function<bool(const std::string)> f, const std::function<std::string(const std::string)> s)
			{
				size_t amount = 0;
				AutoLock luck(saves.m);

				for (auto& i : saves.objs) {
					if (f(i._id)) {
						i._id = s(i._id);
						amount++;
					}
				}
				return amount;
			}

			template<typename T>
			inline size_t SuperResource<T>::set_enabled(const std::function<bool(const std::string)> f, const std::function<bool(const std::string)> s)
			{
				size_t amount = 0;
				AutoLock luck(saves.m);
				for (auto& i : saves.objs) {
					if (f(i._id)) {
						i._enabled = s(i._id);
						amount++;
					}
				}
				return amount;
			}

			template<typename T>
			inline Resource<T> SuperResource<T>::create(const std::string new_id, const bool enable)
			{
				Resource<T> ret;
				if (get(new_id, ret, true)) return ret;

				internal_data<T> new_data;
				new_data._id = new_id;
				new_data._enabled = enable;
				new_data._data = std::make_shared<T>();

				saves.m.lock();
				saves.objs.push_back(new_data);
				saves.m.unlock();

				return new_data._data;
			}

			template<typename T>
			inline Resource<T> SuperResource<T>::load(const std::string new_id, const bool enable)
			{
				Resource<T> ret;
				if (get(new_id, ret, true)) return ret;

				internal_data<T> new_data;
				new_data._id = new_id;
				new_data._enabled = enable;
				new_data._data = std::make_shared<T>();

				saves.m.lock();
				saves.objs.push_back(new_data);
				saves.m.unlock();

				return new_data._data;
			}

			template<typename T>
			inline bool SuperResource<T>::get(const std::string id, Resource<T>& sptr, const bool also_disabled)
			{
				bool good = false;
				AutoLock luck(saves.m);
				for (auto& i : saves.objs) {
					if (i._id == id && (i._enabled || also_disabled)) {
						sptr = i._data;
						good = true;
						break;
					}
				}
				return good;
			}

			template<typename T>
			inline bool SuperResource<T>::remove(const std::string id)
			{
				bool good = false;
				AutoLock luck(saves.m);
				for (size_t p = 0; p < saves.objs.size(); p++) {
					auto& i = saves.objs[p];
					if (i._id == id) {
						saves.objs.erase(saves.objs.begin() + p);
						good = true;
						break;
					}
				}
				return good;
			}

			template<typename T>
			inline bool SuperResource<T>::remove(const Resource<T> sptr)
			{
				bool good = false;
				AutoLock luck(saves.m);
				for (size_t p = 0; p < saves.objs.size(); p++) {
					auto& i = saves.objs[p];
					if (i._data == sptr) {
						saves.objs.erase(saves.objs.begin() + p);
						good = true;
						break;
					}
				}
				return good;
			}

			template<typename T>
			inline std::vector<Resource<T>> SuperResource<T>::get_list(const std::function<bool(const std::string)> f)
			{
				std::vector<Resource<T>> lst;
				AutoLock luck(saves.m);
				for (size_t p = 0; p < saves.objs.size(); p++) {
					auto& i = saves.objs[p];
					if (f(i._id)) {
						lst.push_back(i._data);
					}
				}
				return lst;
			}

			template<typename T>
			inline Resource<T> SuperResource<T>::swap(const std::string id, Resource<T>& nptr)
			{
				AutoLock luck(saves.m);
				for (auto& i : saves.objs) {
					if (i._id == id) {
						i._data.swap(nptr);
					}
				}
				return nptr;
			}

			template<typename T>
			inline void SuperResource<T>::clear()
			{
				AutoLock luck(saves.m);
				saves.objs.clear();
			}

		}
	}
}