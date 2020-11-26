#pragma once

namespace LSW {
	namespace v5 {
		namespace Tools {

			template<typename T>
			template<typename K, typename... Args>
			inline void SuperPair<T>::handleInput(K& ref, Args... args)
			{
				set(ref);
				handleInput(args...);
			}

			template<typename T>
			template<typename K>
			inline void SuperPair<T>::handleInput(K& ref)
			{
				set(ref);
			}

			template<typename T>
			inline SuperPair<T>::SuperPair(const SuperPair& sp)
			{
				*this = sp;
			}

			template<typename T>
			inline SuperPair<T>::SuperPair(SuperPair&& sp) noexcept
			{
				*this = std::move(sp);
			}

			template<typename T>
			template<typename... Args>
			inline SuperPair<T>::SuperPair(const T& val, Args... args)
			{
				holding = val;
				handleInput(args...);
			}

			template<typename T>
			inline SuperPair<T>::SuperPair(const T& val)
			{
				holding = val;
			}

			template<typename T>
			inline void SuperPair<T>::operator=(const SuperPair& sp)
			{
				keys = sp.keys;
				holding = sp.holding;
			}

			template<typename T>
			inline void SuperPair<T>::operator=(SuperPair&& sp) noexcept
			{
				keys = std::move(sp.keys);
				holding = std::move(sp.holding);
			}

			template<typename T>
			inline void SuperPair<T>::set_value(const T& nval)
			{
				holding = nval;
			}

			template<typename T>
			inline T& SuperPair<T>::get_value()
			{
				return holding;
			}

			template<typename T>
			inline const T& SuperPair<T>::get_value() const
			{
				return holding;
			}

			template<typename T>
			template<typename Q, std::enable_if_t<!std::is_same_v<r_cast_t<Q>, char*>, int>>
			inline void SuperPair<T>::set(const Q& nkey)
			{
				for (auto& i : keys) {
					if (i.type() == typeid(r_cast_t<Q>(nkey))) {
						i = r_cast_t<Q>(nkey);
					}
				}
				std::any lol = r_cast_t<Q>(nkey);
				keys.push_back(lol);
			}

			template<typename T>
			inline void SuperPair<T>::set(const char* nkey)
			{
				set((char*)nkey);
			}

			template<typename T>
			inline void SuperPair<T>::set(char* nkey)
			{
				for (auto& i : keys) {
					if (i.type() == typeid(std::string)) {
						i = std::string(nkey);
					}
				}
				std::any lol = std::string(nkey);
				keys.push_back(lol);
			}

			template<typename T>
			template<typename Q, std::enable_if_t<!std::is_same_v<r_cast_t<Q>, char*>, int>>
			inline T* SuperPair<T>::operator[](const Q& key)
			{
				for (auto& i : keys) {
					if (i.type() == typeid(key)) {
						if (std::any_cast<r_cast_t<Q>>(i) == key)
							return &holding;
					}
				}
				return nullptr;
			}

			template<typename T>
			inline T* SuperPair<T>::operator[](const char* key)
			{
				return (*this)[(char*)key];
			}

			template<typename T>
			inline T* SuperPair<T>::operator[](char* key)
			{
				for (auto& i : keys) {
					if (i.type() == typeid(std::string)) {
						if (std::any_cast<std::string>(i) == key)
							return &holding;
					}
				}
				return nullptr;
			}

			template<typename T>
			template<typename Q, std::enable_if_t<!std::is_same_v<r_cast_t<Q>, char*>, int>>
			inline const T* SuperPair<T>::operator[](const Q& key) const
			{
				for (auto& i : keys) {
					if (i.type() == typeid(key)) {
						if (std::any_cast<r_cast_t<Q>>(i) == key)
							return &holding;
					}
				}
				return nullptr;
			}

			template<typename T>
			inline const T* SuperPair<T>::operator[](const char* key) const
			{
				return (*this)[(char*)key];
			}

			template<typename T>
			inline const T* SuperPair<T>::operator[](char* key) const
			{
				for (auto& i : keys) {
					if (i.type() == typeid(std::string)) {
						if (std::any_cast<std::string>(i) == key)
							return &holding;
					}
				}
				return nullptr;
			}

			template<typename T>
			template<typename Q>
			inline void SuperPair<T>::remove()
			{
				for (size_t p = 0; p < keys.size(); p++) {
					if (keys[p].type() == typeid(r_cast_t<Q>))
					{
						keys.erase(keys.begin() + p);
						return;
					}
				}
			}

			template<typename T>
			template<typename Q>
			inline bool SuperPair<T>::has_type() const
			{
				for (auto& i : keys) if (i.type() == typeid(r_cast_t<Q>)) return true;
				return false;
			}

			template<typename T>
			template<typename Q, std::enable_if_t<!std::is_same_v<r_cast_t<Q>, char*>, int>>
			inline bool SuperPair<T>::has_type(const Q& key) const
			{
				for (auto& i : keys) {
					if (i.type() == typeid(key)) {
						if (std::any_cast<r_cast_t<Q>>(i) == key) return true;
					}
				}
				return false;
			}

			template<typename T>
			inline bool SuperPair<T>::has_type(const char* key) const
			{
				return has_type(std::string(key));
			}

			template<typename T>
			inline bool SuperPair<T>::has_type(char* key) const
			{
				return has_type(std::string(key));
			}

			template<typename T>
			template<typename K>
			inline bool SuperPair<T>::get_type(K& val) const
			{
				for (auto& i : keys) {
					if (i.type() == typeid(r_cast_t<K>))
					{
						val = std::any_cast<K>(i);
						return true;
					}
				}
				return false;
			}

		}
	}
}