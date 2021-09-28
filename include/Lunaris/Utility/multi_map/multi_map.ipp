#pragma once

namespace Lunaris {

    template<typename ...T>
    template<typename V, typename Orig>
    inline const V& multiple_data<T...>::operator=(const V& oth)
    {
        return; // none
    }

    template<typename ...T>
    template<typename V, typename Orig>
    inline void multiple_data<T...>::operator=(V&& oth)
    {
        return; // none
    }

    template<typename ...T>
    template<typename V, typename Orig>
    inline bool multiple_data<T...>::operator==(const V& oth) const
    {
        return false;
    }

    template<typename ...T>
    inline bool multiple_data<T...>::operator==(const multiple_data& oth) const
    {
        return false; // this is or, and if none was true, this shouldn't 
    }

    template<typename ...T>
    inline bool multiple_data<T...>::is_one_equal(const multiple_data& oth) const
    {
        return false; // this is or, and if none was true, this shouldn't 
    }

    template<typename ...T>
    inline bool multiple_data<T...>::is_all_equal(const multiple_data& oth) const
    {
        return true; // this is and, so if all true, this must be true
    }

    template<typename ...T>
    template<typename V, typename Orig>
    inline multiple_data<T...>::operator V() const
    {
        throw std::runtime_error("Invalid type casting!");
        return *((V*)nullptr);
    }

    template<typename T, typename ...Rest>
    inline multiple_data<T, Rest...>::multiple_data(const T& first, const Rest & ...rest)
        : first(first), rest(rest...)
    {
    }

    template<typename T, typename ...Rest>
    template<typename V, typename Orig>
    inline const V& multiple_data<T, Rest...>::operator=(const V& oth)
    {
        if (typeid(Orig) == typeid(first)) {
            (*(V*)&first) = oth;
            return oth;
        }
        return rest = oth;
    }

    template<typename T, typename ...Rest>
    template<typename V, typename Orig>
    inline void multiple_data<T, Rest...>::operator=(V&& oth)
    {
        if (typeid(Orig) == typeid(first)) {
            (*(V*)&first) = std::move(oth);
            return;
        }
        return rest = std::move(oth);
    }

    template<typename T, typename ...Rest>
    template<typename V, typename Orig>
    inline bool multiple_data<T, Rest...>::operator==(const V& oth) const
    {
        if (typeid(Orig) == typeid(first)) return (*(Orig*)&first) == oth;
        return rest == oth;
    }

    template<typename T, typename ...Rest>
    inline bool multiple_data<T, Rest...>::operator==(const multiple_data& oth) const
    {
        return first == oth.first || rest == oth.rest;
    }

    template<typename T, typename ...Rest>
    inline bool multiple_data<T, Rest...>::is_one_equal(const multiple_data& oth) const
    {
        return first == oth.first || rest == oth.rest;
    }

    template<typename T, typename ...Rest>
    inline bool multiple_data<T, Rest...>::is_all_equal(const multiple_data& oth) const
    {
        return first == oth.first && rest == oth.rest;
    }

    template<typename T, typename ...Rest>
    template<typename V, typename Orig>
    inline multiple_data<T, Rest...>::operator V() const
    {
        if (typeid(Orig) == typeid(first))
            return (*(V*)&first);
        V test = static_cast<V>(rest);
        return test;
    }


    template<typename Store, typename ...Keys>
    inline multi_pair<Store, Keys...>::multi_pair(const Store& stor, const Keys&... vals)
        : store(stor), keys(vals...)
    {
    }


    template<typename Store, size_t amount, typename ...Keys>
    inline bool fixed_multi_map<Store, amount, Keys...>::generic_assign(const multi_pair<Store, Keys...>& oth)
    {
        for (auto& i : objects) {
            if (i.keys == oth.keys) { // has one equal
                i = oth;
                return true;
            }
        }
        return false;
    }

    template<typename Store, size_t amount, typename ...Keys>
    inline bool fixed_multi_map<Store, amount, Keys...>::generic_assign(multi_pair<Store, Keys...>&& oth)
    {
        for (auto& i : objects) {
            if (i.keys == oth.keys) { // has one equal
                i = std::move(oth);
                return true;
            }
        }
        return false;
    }

    template<typename Store, size_t amount, typename ...Keys>
    inline fixed_multi_map<Store, amount, Keys...>::fixed_multi_map(const fixed_multi_map& oth)
    {
        for (size_t p = 0; p < amount; p++) objects[p] = oth.objects[p];
    }

    template<typename Store, size_t amount, typename ...Keys>
    inline fixed_multi_map<Store, amount, Keys...>::fixed_multi_map(fixed_multi_map&& oth) noexcept
    {
        for (size_t p = 0; p < amount; p++) objects[p] = std::move(oth.objects[p]);
    }

    template<typename Store, size_t amount, typename ...Keys>
    inline fixed_multi_map<Store, amount, Keys...>::fixed_multi_map(std::initializer_list<multi_pair<Store, Keys...>> list)
    {
        if (list.size() != amount) throw std::runtime_error("Initializer list is not of the same size of the fixed sized multimap!");
        for (size_t p = 0; p < amount; p++) objects[p] = *(list.begin() + p);
    }

    template<typename Store, size_t amount, typename ...Keys>
    inline auto fixed_multi_map<Store, amount, Keys...>::begin()
    {
        return std::begin(objects);
    }

    template<typename Store, size_t amount, typename ...Keys>
    inline auto fixed_multi_map<Store, amount, Keys...>::end()
    {
        return std::end(objects);
    }

    template<typename Store, size_t amount, typename ...Keys>
    inline auto fixed_multi_map<Store, amount, Keys...>::begin() const
    {
        return std::cbegin(objects);
    }

    template<typename Store, size_t amount, typename ...Keys>
    inline auto fixed_multi_map<Store, amount, Keys...>::end() const
    {
        return std::cend(objects);
    }

    template<typename Store, size_t amount, typename ...Keys>
    inline size_t fixed_multi_map<Store, amount, Keys...>::size() const
    {
        return amount;
    }

    template<typename Store, size_t amount, typename ...Keys>
    template<typename V>
    inline Store& fixed_multi_map<Store, amount, Keys...>::at(const V& oth)
    {
        for (auto& i : objects) {
            if (i.keys == oth) return i.store;
        }
        throw std::out_of_range("no key found");
        return objects[0].store; // no "not all paths return" even though this won't ever be called
    }

    template<typename Store, size_t amount, typename ...Keys>
    template<typename V>
    inline const Store& fixed_multi_map<Store, amount, Keys...>::at(const V& oth) const
    {
        for (auto& i : objects) {
            if (i.keys == oth) return i.store;
        }
        throw std::out_of_range("no key found");
        return objects[0].store; // no "not all paths return" even though this won't ever be called
    }

    template<typename Store, size_t amount, typename ...Keys>
    template<typename V>
    inline Store& fixed_multi_map<Store, amount, Keys...>::operator[](const V& oth)
    {
        for (auto& i : objects) {
            if (i.keys == oth) return i.store;
        }
        throw std::out_of_range("no key found");
        return objects[0].store; // no "not all paths return" even though this won't ever be called
    }

    template<typename Store, size_t amount, typename ...Keys>
    template<typename V>
    inline const Store& fixed_multi_map<Store, amount, Keys...>::operator[](const V& oth) const
    {
        for (auto& i : objects) {
            if (i.keys == oth) return i.store;
        }
        throw std::out_of_range("no key found");
        return objects[0].store; // no "not all paths return" even though this won't ever be called
    }

    template<typename Store, size_t amount, typename ...Keys>
    template<typename V>
    inline auto fixed_multi_map<Store, amount, Keys...>::find(const V& oth)
    {
        return std::find_if(begin(), end(), [&](const multi_pair<Store, Keys...>& a) { return a.keys == oth; });
    }

    template<typename Store, size_t amount, typename ...Keys>
    template<typename V>
    inline auto fixed_multi_map<Store, amount, Keys...>::find(const V& oth) const
    {
        return std::find_if(begin(), end(), [&](const multi_pair<Store, Keys...>& a) { return a.keys == oth; });
    }

    template<typename Store, size_t amount, typename ...Keys>
    template<typename V>
    inline bool fixed_multi_map<Store, amount, Keys...>::contains(const V& oth) const
    {
        return std::find_if(begin(), end(), [&](const multi_pair<Store, Keys...>& a) { return a.keys == oth; }) != end();
    }

    template<typename Store, size_t amount, typename ...Keys>
    inline auto fixed_multi_map<Store, amount, Keys...>::find_if(find_func func)
    {
        return std::find_if(begin(), end(), func);
    }

    template<typename Store, size_t amount, typename ...Keys>
    inline auto fixed_multi_map<Store, amount, Keys...>::find_if(find_func func) const
    {
        return std::find_if(begin(), end(), func);
    }


    template<typename Store, typename ...Keys>
    inline void multi_map<Store, Keys...>::generic_add_or_assign(const multi_pair<Store, Keys...>& oth)
    {
        for (auto& i : objects) {
            if (i.keys == oth.keys) { // has one equal
                i = oth;
                return;
            }
        }
        objects.push_back(oth);
    }

    template<typename Store, typename ...Keys>
    inline void multi_map<Store, Keys...>::generic_add_or_assign(multi_pair<Store, Keys...>&& oth)
    {
        for (auto& i : objects) {
            if (i.keys == oth.keys) { // has one equal
                i = std::move(oth);
                return;
            }
        }
        objects.emplace_back(std::move(oth));
    }

    template<typename Store, typename... Keys>
    inline multi_map<Store, Keys...>::multi_map(const multi_map& oth)
        : objects(oth.objects)
    {
    }

    template<typename Store, typename... Keys>
    inline multi_map<Store, Keys...>::multi_map(multi_map&& oth) noexcept
        : objects(std::move(oth.objects))
    {
    }

    template<typename Store, typename... Keys>
    inline multi_map<Store, Keys...>::multi_map(std::initializer_list<multi_pair<Store, Keys...>> list)
    {
        for (const auto& i : list) objects.push_back(i);
    }

    template<typename Store, typename... Keys>
    inline const multi_map<Store, Keys...>& multi_map<Store, Keys...>::operator=(const multi_map& oth)
    {
        objects = oth.objects;
    }

    template<typename Store, typename... Keys>
    inline const multi_map<Store, Keys...>& multi_map<Store, Keys...>::operator=(multi_map&& oth) noexcept
    {
        objects = std::move(oth.objects);
    }

    template<typename Store, typename ...Keys>
    inline const multi_map<Store, Keys...>& multi_map<Store, Keys...>::operator+=(const multi_map& oth)
    {
        for (const auto& i : oth.objects) generic_add_or_assign(i);
        return *this;
    }

    template<typename Store, typename ...Keys>
    inline const multi_map<Store, Keys...>& multi_map<Store, Keys...>::operator+=(multi_map&& oth) noexcept
    {
        for (auto& i : oth.objects) generic_add_or_assign(std::move(i));
        oth.objects.clear();
        return *this;
    }

    template<typename Store, typename ...Keys>
    inline const multi_map<Store, Keys...>& multi_map<Store, Keys...>::operator+=(const multi_pair<Store, Keys...>& oth)
    {
        generic_add_or_assign(oth);
        return *this;
    }

    template<typename Store, typename ...Keys>
    inline const multi_map<Store, Keys...>& multi_map<Store, Keys...>::operator+=(multi_pair<Store, Keys...>&& oth) noexcept
    {
        generic_add_or_assign(std::move(oth));
        return *this;
    }

    template<typename Store, typename... Keys>
    inline typename multi_map<Store, Keys...>::multi_map_iterator multi_map<Store, Keys...>::begin()
    {
        return objects.begin();
    }

    template<typename Store, typename... Keys>
    inline typename multi_map<Store, Keys...>::multi_map_iterator multi_map<Store, Keys...>::end()
    {
        return objects.end();
    }

    template<typename Store, typename... Keys>
    inline typename multi_map<Store, Keys...>::const_multi_map_iterator multi_map<Store, Keys...>::begin() const
    {
        return objects.cbegin();
    }

    template<typename Store, typename... Keys>
    inline typename multi_map<Store, Keys...>::const_multi_map_iterator multi_map<Store, Keys...>::end() const
    {
        return objects.cend();
    }

    template<typename Store, typename... Keys>
    inline bool multi_map<Store, Keys...>::empty() const
    {
        return objects.empty();
    }

    template<typename Store, typename... Keys>
    inline size_t multi_map<Store, Keys...>::size() const
    {
        return objects.size();
    }

    template<typename Store, typename... Keys>
    inline size_t multi_map<Store, Keys...>::max_size() const
    {
        return objects.max_size();
    }

    template<typename Store, typename... Keys>
    inline void multi_map<Store, Keys...>::clear()
    {
        objects.clear();
    }

    template<typename Store, typename... Keys>
    inline void multi_map<Store, Keys...>::insert(const multi_pair<Store, Keys...>& oth)
    {
        generic_add_or_assign(oth);
    }

    template<typename Store, typename... Keys>
    inline void multi_map<Store, Keys...>::insert(multi_pair<Store, Keys...>&& oth)
    {
        generic_add_or_assign(std::move(oth));
    }

    template<typename Store, typename... Keys>
    inline void multi_map<Store, Keys...>::insert(std::initializer_list<multi_pair<Store, Keys...>> list)
    {
        for(const auto& i : list) generic_add_or_assign(i);
        return *this;
    }

    template<typename Store, typename... Keys>
    inline void multi_map<Store, Keys...>::push_back(const multi_pair<Store, Keys...>& oth)
    {
        generic_add_or_assign(oth);
    }

    template<typename Store, typename... Keys>
    inline void multi_map<Store, Keys...>::push_back(multi_pair<Store, Keys...>&& oth)
    {
        generic_add_or_assign(std::move(oth));
    }

    template<typename Store, typename... Keys>
    inline void multi_map<Store, Keys...>::push_back(std::initializer_list<multi_pair<Store, Keys...>> list)
    {
        for (const auto& i : list) generic_add_or_assign(i);
        return *this;
    }

    template<typename Store, typename... Keys>
    inline void multi_map<Store, Keys...>::erase(typename multi_map_iterator it)
    {
        objects.erase(it);
    }

    template<typename Store, typename... Keys>
    inline void multi_map<Store, Keys...>::erase(typename const_multi_map_iterator it)
    {
        objects.erase(it);
    }

    template<typename Store, typename... Keys>
    inline void multi_map<Store, Keys...>::erase(typename const_multi_map_iterator first, typename const_multi_map_iterator last)
    {
        objects.erase(first, last);
    }

    template<typename Store, typename... Keys>
    template<typename V>
    inline void multi_map<Store, Keys...>::erase(const V& key)
    {
        const_multi_map_iterator it = std::find_if(objects.begin(), objects.end(), [&](const multi_pair<Store, Keys...>& a) { return a == key; });
        if (it != objects.end()) objects.erase(it);
    }

    template<typename Store, typename... Keys>
    inline void multi_map<Store, Keys...>::swap(multi_map& oth)
    {
        objects.swap(oth.objects);
    }

    template<typename Store, typename... Keys>
    template<typename V>
    inline Store& multi_map<Store, Keys...>::at(const V& oth)
    {
        for (auto& i : objects) {
            if (i.keys == oth) return i.store;
        }
        throw std::out_of_range("no key found");
        return objects[0].store; // no "not all paths return" even though this won't ever be called
    }

    template<typename Store, typename... Keys>
    template<typename V>
    inline const Store& multi_map<Store, Keys...>::at(const V& oth) const
    {
        for (const auto& i : objects) {
            if (i.keys == oth) return i.store;
        }
        throw std::out_of_range("no key found");
        return objects[0].store; // no "not all paths return" even though this won't ever be called
    }

    template<typename Store, typename... Keys>
    template<typename V>
    inline Store& multi_map<Store, Keys...>::operator[](const V& oth)
    {
        for (auto& i : objects) {
            if (i.keys == oth) return i.store;
        }
        throw std::out_of_range("no key found");
        return objects[0].store; // no "not all paths return" even though this won't ever be called
    }

    template<typename Store, typename... Keys>
    template<typename V>
    inline const Store& multi_map<Store, Keys...>::operator[](const V& oth) const
    {
        for (const auto& i : objects) {
            if (i.keys == oth) return i.store;
        }
        throw std::out_of_range("no key found");
        return objects[0].store; // no "not all paths return" even though this won't ever be called
    }

    template<typename Store, typename... Keys>
    template<typename V>
    inline typename multi_map<Store, Keys...>::multi_map_iterator multi_map<Store, Keys...>::find(const V& oth)
    {
        return std::find_if(objects.begin(), objects.end(), [&](const multi_pair<Store, Keys...>& a) { return a.keys == oth; });
    }

    template<typename Store, typename... Keys>
    template<typename V>
    inline typename multi_map<Store, Keys...>::const_multi_map_iterator multi_map<Store, Keys...>::find(const V& oth) const
    {
        return std::find_if(objects.cbegin(), objects.cend(), [&](const multi_pair<Store, Keys...>& a) { return a.keys == oth; });
    }

    template<typename Store, typename... Keys>
    template<typename V>
    inline bool multi_map<Store, Keys...>::contains(const V& oth) const
    {
        return std::find_if(objects.cbegin(), objects.cend(), [&](const multi_pair<Store, Keys...>& a) { return a.keys == oth; }) != objects.cend();
    }

    template<typename Store, typename... Keys>
    inline typename multi_map<Store, Keys...>::multi_map_iterator multi_map<Store, Keys...>::find_if(find_func func)
    {
        return std::find_if(objects.begin(), objects.end(), func);
    }

    template<typename Store, typename... Keys>
    inline typename multi_map<Store, Keys...>::const_multi_map_iterator multi_map<Store, Keys...>::find_if(find_func func) const
    {
        return std::find_if(objects.cbegin(), objects.cend(), func);
    }



    template<typename MultiMap, typename Hint, typename ...Types>
    template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int>>
    inline Hint& __multi_map_work<MultiMap, Hint, Types...>::get(const O& oth)
    {
        return this->at(oth);
    }

    template<typename MultiMap, typename Hint, typename ...Types>
    template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int>>
    inline const Hint& __multi_map_work<MultiMap, Hint, Types...>::get(const O& oth) const
    {
        return this->at(oth);
    }

    template<typename MultiMap, typename Hint, typename ...Types>
    template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int>>
    inline void __multi_map_work<MultiMap, Hint, Types...>::set(const O& oth, const Hint& val)
    {
        this->at(oth) = val;
    }

    template<typename MultiMap, typename Hint, typename ...Types>
    template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int>>
    inline auto& __multi_map_work<MultiMap, Hint, Types...>::index(const size_t pos)
    {
        if (pos >= size()) throw std::runtime_error("Invalid index position!");
        return *(this->begin() + pos);
    }

    template<typename MultiMap, typename Hint, typename ...Types>
    template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int>>
    inline size_t __multi_map_work<MultiMap, Hint, Types...>::size()
    {
        return size();
    }

    template<typename MultiMap, typename Hint, typename ...Types>
    template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int>>
    inline typename MultiMap& __multi_map_work<MultiMap, Hint, Types...>::self()
    {
        return *this;
    }

    template<typename MultiMap, size_t siz, typename Hint, typename ...Types>
    template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int>>
    inline Hint& __fixed_multi_map_work<MultiMap, siz, Hint, Types...>::get(const O& oth)
    {
        return this->at(oth);
    }

    template<typename MultiMap, size_t siz, typename Hint, typename ...Types>
    template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int>>
    inline const Hint& __fixed_multi_map_work<MultiMap, siz, Hint, Types...>::get(const O& oth) const
    {
        return this->at(oth);
    }

    template<typename MultiMap, size_t siz, typename Hint, typename ...Types>
    template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int>>
    inline void __fixed_multi_map_work<MultiMap, siz, Hint, Types...>::set(const O& oth, const Hint& val)
    {
        this->at(oth) = val;
    }

    template<typename MultiMap, size_t siz, typename Hint, typename ...Types>
    template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int>>
    inline auto& __fixed_multi_map_work<MultiMap, siz, Hint, Types...>::index(const size_t pos)
    {
        if (pos >= siz) throw std::runtime_error("Invalid index position!");
        return *(this->begin() + pos);
    }

    template<typename MultiMap, size_t siz, typename Hint, typename ...Types>
    template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int>>
    inline size_t __fixed_multi_map_work<MultiMap, siz, Hint, Types...>::size()
    {
        return siz;
    }

    template<typename MultiMap, size_t siz, typename Hint, typename ...Types>
    template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int>>
    inline typename MultiMap& __fixed_multi_map_work<MultiMap, siz, Hint, Types...>::self()
    {
        return *this;
    }

}