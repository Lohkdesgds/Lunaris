#pragma once

#include <vector>
#include <typeinfo>
#include <stdexcept>

namespace Lunaris {

    template<typename T> using r_cast_t = std::conditional_t<std::is_pointer<T>::value || std::is_array<T>::value, std::add_pointer_t<std::remove_all_extents_t<std::remove_cv_t<std::remove_reference_t<std::remove_pointer_t<T>>>>>, std::remove_all_extents_t<std::remove_cv_t<std::remove_reference_t<std::remove_pointer_t<T>>>>>;

    // Reference: https://stackoverflow.com/questions/29671643/checking-type-of-parameter-pack-using-enable-if
    template<bool...> struct bool_pack;
    template<bool... bs>
    using all_true = std::is_same<bool_pack<bs..., true>, bool_pack<true, bs...>>;
    template<class R, class... Ts>
    using are_all_convertible = all_true<std::is_convertible<Ts, R>::value...>;

    template<typename... T>
    class multiple_data {
    public:
        template<typename V, typename Orig = r_cast_t<V>>
        const V& operator=(const V&);

        template<typename V, typename Orig = r_cast_t<V>>
        void operator=(V&&);

        template<typename V, typename Orig = r_cast_t<V>>
        bool operator==(const V&) const;

        // by default, if one equal
        bool operator==(const multiple_data&) const;

        bool is_one_equal(const multiple_data&) const;
        bool is_all_equal(const multiple_data&) const;

        template<typename V, typename Orig = r_cast_t<V>>
        operator V() const;
    };

    template<typename T, typename... Rest>
    class multiple_data<T, Rest...> {
        T first{};
        multiple_data<Rest...> rest;
    public:
        multiple_data() = default;
        multiple_data(const T& first, const Rest&... rest);

        template<typename V, typename Orig = r_cast_t<V>>
        const V& operator=(const V&);

        template<typename V, typename Orig = r_cast_t<V>>
        void operator=(V&&);

        template<typename V, typename Orig = r_cast_t<V>>
        bool operator==(const V&) const;

        // by default, if one equal
        bool operator==(const multiple_data&) const;

        bool is_one_equal(const multiple_data&) const;
        bool is_all_equal(const multiple_data&) const;

        template<typename V, typename Orig = r_cast_t<V>>
        operator V() const;
    };

    template<typename Store, typename... Keys>
    struct multi_pair {
        Store store{};
        multiple_data<Keys...> keys;

        multi_pair() = default;
        multi_pair(const Store&, const Keys&...);
    };
    

    template<typename Store, size_t amount, typename... Keys>
    class fixed_multi_map {
        multi_pair<Store, Keys...> objects[amount];
    public:        
        typedef bool(*find_func)(const multi_pair<Store, Keys...>&);
    private:
        bool generic_assign(const multi_pair<Store, Keys...>&);
        bool generic_assign(multi_pair<Store, Keys...>&&);
    public:
        fixed_multi_map() = default;

        fixed_multi_map(const fixed_multi_map&);
        fixed_multi_map(fixed_multi_map&&) noexcept;
        fixed_multi_map(std::initializer_list<multi_pair<Store, Keys...>>);

        auto begin();
        auto end();
        auto begin() const;
        auto end() const;

        size_t size() const;

        template<typename V> Store& at(const V&);
        template<typename V> const Store& at(const V&) const;
        template<typename V> Store& operator[](const V&);
        template<typename V> const Store& operator[](const V&) const;

        template<typename V> auto find(const V&);
        template<typename V> auto find(const V&) const;

        template<typename V> bool contains(const V&) const;

        auto find_if(find_func);
        auto find_if(find_func) const;
    };


    template<typename Store, typename... Keys>
    class multi_map {
    public:
        using multi_map_iterator = typename std::vector<multi_pair<Store, Keys...>>::iterator;
        using const_multi_map_iterator = typename std::vector<multi_pair<Store, Keys...>>::const_iterator;
        typedef bool(*find_func)(const multi_pair<Store, Keys...>&);

    private:
        std::vector<multi_pair<Store, Keys...>> objects;

        void generic_add_or_assign(const multi_pair<Store, Keys...>&);
        void generic_add_or_assign(multi_pair<Store, Keys...>&&);
    public:
        multi_map() = default;

        multi_map(const multi_map&);
        multi_map(multi_map&&) noexcept;
        multi_map(std::initializer_list<multi_pair<Store, Keys...>>);

        const multi_map& operator=(const multi_map&);
        const multi_map& operator=(multi_map&&) noexcept;

        const multi_map& operator+=(const multi_map&);
        const multi_map& operator+=(multi_map&&) noexcept;

        const multi_map& operator+=(const multi_pair<Store, Keys...>&);
        const multi_map& operator+=(multi_pair<Store, Keys...>&&) noexcept;

        multi_map_iterator begin();
        multi_map_iterator end();
        const_multi_map_iterator begin() const;
        const_multi_map_iterator end() const;

        bool empty() const;
        size_t size() const;
        size_t max_size() const;

        void clear();
        void insert(const multi_pair<Store, Keys...>&);
        void insert(multi_pair<Store, Keys...>&&);
        void insert(std::initializer_list<multi_pair<Store, Keys...>>);

        void push_back(const multi_pair<Store, Keys...>&);
        void push_back(multi_pair<Store, Keys...>&&);
        void push_back(std::initializer_list<multi_pair<Store, Keys...>>);

        void erase(multi_map_iterator);
        void erase(const_multi_map_iterator);
        void erase(const_multi_map_iterator, const_multi_map_iterator);
        template<typename V> void erase(const V&);

        void swap(multi_map<Store, Keys...>&);

        template<typename V> Store& at(const V&);
        template<typename V> const Store& at(const V&) const;
        template<typename V> Store& operator[](const V&);
        template<typename V> const Store& operator[](const V&) const;

        template<typename V> multi_map_iterator find(const V&);
        template<typename V> const_multi_map_iterator find(const V&) const;

        template<typename V> bool contains(const V&) const;

        multi_map_iterator find_if(find_func);
        const_multi_map_iterator find_if(find_func) const;
    };

    template<typename MultiMap, typename Hint, typename... Types>
    class __multi_map_work : public MultiMap {
    public:
        using MultiMap::MultiMap;

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0>
        Hint& get(const O&);

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0>
        const Hint& get(const O&) const;

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0>
        void set(const O&, const Hint&);

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0> // O is necessary like a key so many hierarchy of same return type don't collide
        auto& index(const size_t);

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0> // O is necessary like a key so many hierarchy of same return type don't collide
        size_t size();

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0> // O is necessary like a key so many hierarchy of same return type don't collide
        typename MultiMap& self();
    };

    template<typename MultiMap, size_t siz, typename Hint, typename... Types>
    class __fixed_multi_map_work : public MultiMap {
    public:
        using MultiMap::MultiMap;

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0>
        Hint& get(const O&);

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0>
        const Hint& get(const O&) const;

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0>
        void set(const O&, const Hint&);

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0> // O is necessary like a key so many hierarchy of same return type don't collide
        auto& index(const size_t);

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0> // O is necessary like a key so many hierarchy of same return type don't collide
        size_t size();

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0> // O is necessary like a key so many hierarchy of same return type don't collide
        typename MultiMap& self();
    };

    template<template <class, size_t, class...> class Class, size_t Size, typename Hint, typename... Keys>
    using generic_fixed_multi_map_work = __fixed_multi_map_work<Class<Hint, Size, Keys...>, Size, Hint, Keys...>;

    template<template <class, class...> class Class, typename Hint, typename... Keys>
    using generic_multi_map_work = __multi_map_work<Class<Hint, Keys...>, Hint, Keys...>;

    template<size_t Size, typename Hint, typename... Keys>
    using fixed_multi_map_work = __fixed_multi_map_work<fixed_multi_map<Hint, Size, Keys...>, Size, Hint, Keys...>;

    template<typename Hint, typename... Keys>
    using multi_map_work = __multi_map_work<multi_map<Hint, Keys...>, Hint, Keys...>;


    /*template<template <typename, typename...> class MultiMap, typename Hint, typename... Types>
    class __multi_map_work : protected MultiMap<Hint, Types...> {
    public:
        using MultiMap<Hint, Types...>::MultiMap;

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0>
        Hint& get(const O&);

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0>
        const Hint& get(const O&) const;

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0>
        void set(const O&, const Hint&);

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0> // O is necessary like a key so many hierarchy of same return type don't collide
        auto& index(const size_t);

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0> // O is necessary like a key so many hierarchy of same return type don't collide
        size_t size();

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0> // O is necessary like a key so many hierarchy of same return type don't collide
        typename MultiMap<Hint, Types...>& self();
    };

    template<template <typename, size_t, typename...> class MultiMap, size_t siz, typename Hint, typename... Types>
    class __fixed_multi_map_work : protected MultiMap<Hint, siz, Types...> {
    public:
        using MultiMap<Hint, siz, Types...>::MultiMap;

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0>
        Hint& get(const O&);

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0>
        const Hint& get(const O&) const;

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0>
        void set(const O&, const Hint&);

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0> // O is necessary like a key so many hierarchy of same return type don't collide
        auto& index(const size_t);

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0> // O is necessary like a key so many hierarchy of same return type don't collide
        size_t size();

        template<typename V, typename O, std::enable_if_t<(std::is_same_v<V, Hint> && (std::disjunction_v<std::is_same<O, Types>...>)), int> = 0> // O is necessary like a key so many hierarchy of same return type don't collide
        typename MultiMap<Hint, siz, Types...>& self();
    };*/

}

#include "multi_map.ipp"