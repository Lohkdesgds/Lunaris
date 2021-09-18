#pragma once


namespace LSW {
	namespace v5 {
		namespace Tools {

			template<typename BaseType>
			inline SuperFunctionalMap<BaseType>::SuperFunctionalMap(const SuperMap<FastFunction<BaseType>>& olmap)
			{
				(*map) = olmap;
			}

			template<typename BaseType>
			template<typename Key, typename Compare, std::enable_if_t<std::is_same<Compare, BaseType>::value, int>>
			inline std::shared_ptr<SuperMap<FastFunction<BaseType>>> SuperFunctionalMap<BaseType>::get()
			{
				return map;
			}

			template<typename BaseType>
			template<typename Key, typename Compare, std::enable_if_t<std::is_same<Compare, BaseType>::value, int>>
			inline std::shared_ptr<SuperMap<FastFunction<BaseType>>> SuperFunctionalMap<BaseType>::get() const
			{
				return map;
			}

			template<typename BaseType>
			template<typename Key, typename Compare, std::enable_if_t<std::is_same<Compare, BaseType>::value, int>>
			inline void SuperFunctionalMap<BaseType>::set(const SuperMap<FastFunction<BaseType>>& mp)
			{
				map->add(mp);
			}

			template<typename BaseType>
			template<typename Key, typename Compare, std::enable_if_t<std::is_same<Compare, BaseType>::value, int>>
			inline void SuperFunctionalMap<BaseType>::set(SuperMap<FastFunction<BaseType>>&& mp)
			{
				map->add(std::move(mp));
			}

			template<typename BaseType>
			template<typename Key, typename Compare, std::enable_if_t<std::is_same<Compare, BaseType>::value, int>>
			inline void SuperFunctionalMap<BaseType>::set(const std::shared_ptr<SuperMap<FastFunction<BaseType>>>& mp)
			{
				map = mp;
			}

			template<typename BaseType>
			template<typename Key, typename Compare, std::enable_if_t<std::is_same<Compare, BaseType>::value, int>>
			inline void SuperFunctionalMap<BaseType>::set(std::shared_ptr<SuperMap<FastFunction<BaseType>>>&& mp)
			{
				map = std::move(mp);
			}

			template<typename BaseType>
			template<typename Key, typename Arg1, typename Compare,	std::enable_if_t<std::is_same<Compare, BaseType>::value, int>>
			inline void SuperFunctionalMap<BaseType>::set(const Arg1& arg1, const Key& arg2)
			{
				auto* ptr = map->get_pair(arg1);
				if (!ptr) map->add({ FastFunction<BaseType>(arg2), arg1 });
				else ptr->get_value().set(arg2);
			}

			template<typename BaseType>
			template<typename Key, typename Arg1, typename Compare, std::enable_if_t<std::is_same<Compare, BaseType>::value, int>>
			inline void SuperFunctionalMap<BaseType>::set(const Arg1& arg1, std::function<Key(void)> arg2)
			{
				auto* ptr = map->get_pair(arg1);
				if (!ptr) map->add({ FastFunction<BaseType>(arg2), arg1 });
				else ptr->get_value().set(arg2);
			}

			template<typename BaseType>
			template<typename Key, typename Arg1, typename Compare, std::enable_if_t<std::is_same<Compare, BaseType>::value, int>>
			inline bool SuperFunctionalMap<BaseType>::get(const Arg1& arg1, Key& arg2) const
			{
				if (auto* ptr = (*map)[arg1]; ptr) {
					arg2 = (*ptr)();
					return true;
				}
				return false;
			}

			template<typename BaseType>
			template<typename Key, typename Arg1, typename Compare, std::enable_if_t<std::is_same<Compare, BaseType>::value, int>>
			inline bool SuperFunctionalMap<BaseType>::get(const Arg1& arg1, std::function<Key(void)>& arg2) const
			{
				if (auto* ptr = (*map)[arg1]; ptr) {
					arg2 = *ptr;
					return true;
				}
				return false;
			}

			template<typename BaseType>
			template<typename Ret, typename Arg1, typename Compare, std::enable_if_t<(!std::is_pointer<Ret>::value&& std::is_same<Compare, BaseType>::value), int>>
			inline Ret SuperFunctionalMap<BaseType>::get_direct(const Arg1& arg1) const
			{
				if (auto* ptr = (*map)[arg1]; ptr) {
					return (*ptr)();
				}
				return Ret();
			}

			template<typename BaseType>
			template<typename Ret, typename Arg1, typename Compare, std::enable_if_t<(std::is_pointer<Ret>::value&& std::is_same<Compare, BaseType>::value), int>>
			inline std::function<Compare(void)>* SuperFunctionalMap<BaseType>::get_direct(const Arg1& arg1)
			{
				if (auto* ptr = (*map)[arg1]; ptr) {
					return ptr->get_f();
				}
				return nullptr;
			}

			template<typename BaseType>
			template<typename Ret, typename Arg1, typename Compare, std::enable_if_t<(std::is_reference<Ret>::value&& std::is_same<Compare, BaseType>::value), int>>
			inline FastFunction<Compare>* SuperFunctionalMap<BaseType>::get_direct(const Arg1& arg1)
			{
				if (auto* ptr = (*map)[arg1]; ptr) {
					return ptr;
				}
				return nullptr;
			}

		}
	}
}