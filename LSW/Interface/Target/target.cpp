#include "target.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			std::unordered_map<size_t, Tools::FastFunction<Bitmap>> Target::func;

			Target::Target(const size_t i_) : i(i_)
			{
			}

			void Target::set(const Bitmap& c)
			{
				func[i] = c;
			}

			void Target::set(const std::function<Bitmap(void)>& f)
			{
				func[i] = f;
			}

			void Target::apply()
			{
				func[i]().set_as_target();
			}

			Bitmap Target::get()
			{
				return func[i]();
			}

			// implementation

			template Tools::FastFunction<Bitmap>;

		}
	}
}