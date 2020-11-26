#pragma once

namespace LSW {
	namespace v5 {
		namespace Tools {

			template<typename T>
			inline void SuperMap<T>::clear()
			{
				sps.clear();
			}
			
			template<typename T>
			inline SuperMap<T>::SuperMap(std::initializer_list<SuperPair<T>> spTi)
			{
				add(spTi);
			}

			template<typename T>
			inline SuperMap<T>::SuperMap(const SuperMap& mp)
			{
				add(mp);
			}

			template<typename T>
			inline SuperMap<T>::SuperMap(SuperMap&& mp) noexcept
			{
				sps = std::move(mp.sps);
			}

			template<typename T>
			inline void SuperMap<T>::operator=(const SuperMap& mp)
			{
				add(mp);
			}

			template<typename T>
			inline void SuperMap<T>::operator=(SuperMap&& mp) noexcept
			{
				sps = std::move(mp.sps);
			}

			template<typename T>
			inline void SuperMap<T>::add(const SuperMap& mp)
			{
				for (auto& i : mp.sps) {
					SuperPair<T> spp;
					spp = i;
					add(spp);
				}
			}

			template<typename T>
			inline void SuperMap<T>::add(SuperMap&& mp)
			{
				for (auto& i : mp.sps) {
					add(std::move(i));
				}
			}

			template<typename T>
			inline void SuperMap<T>::add(std::initializer_list<SuperPair<T>> spTi)
			{
				for (auto& i : spTi) {
					add(i);
				}
			}

			template<typename T>
			inline void SuperMap<T>::add(const SuperPair<T>& sp)
			{
				sps.push_back(sp);
			}

			template<typename T>
			inline void SuperMap<T>::add(SuperPair<T>&& sp)
			{
				sps.push_back(std::move(sp));
			}

			template<typename T>
			inline auto SuperMap<T>::begin() 
			{
				return sps.begin();
			}

			template<typename T>
			inline auto SuperMap<T>::end() 
			{
				return sps.end();
			}

			template<typename T>
			inline auto SuperMap<T>::begin() const
			{
				return sps.begin();
			}

			template<typename T>
			inline auto SuperMap<T>::end() const
			{
				return sps.end();
			}

			template<typename T>
			inline size_t SuperMap<T>::size() const
			{
				return sps.size();
			}

			template<typename T>
			inline void SuperMap<T>::erase(const size_t start)
			{
				if (start < size()) {
					sps.erase(sps.begin() + start);
				}
			}

			template<typename T>
			inline void SuperMap<T>::erase(const size_t start, const size_t end)
			{
				if (start < end && start + end < size()) {
					sps.erase(sps.begin() + start, sps.begin() + end);
				}
			}

			template<typename T>
			template<typename K>
			inline SuperPair<T>* SuperMap<T>::get_pair(const K& key)
			{
				for (auto& i : sps) if (i.has_type(key)) return &i;
				return nullptr;
			}

			template<typename T>
			template<typename K>
			inline T* SuperMap<T>::operator[](const K& key)
			{
				for (auto& i : sps) {
					if (i.has_type(key)) return i[key];
				}
				return nullptr;
			}

			template<typename T>
			template<typename K>
			inline const T* SuperMap<T>::operator[](const K& key) const
			{
				for (auto& i : sps) {
					if (i.has_type(key)) return i[key];
				}
				return nullptr;
			}

		}
	}
}