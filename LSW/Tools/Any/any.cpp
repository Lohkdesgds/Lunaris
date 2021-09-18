#include "any.h"

namespace LSW {
	namespace v5 {
		namespace Tools {

			Any& Any::operator=(const char* s)
			{
				*this = std::string(s);
				return *this;
			}

			bool Any::operator==(const Any& a) const
			{
				return type() == a.type() && memcmp(&a, this, sizeof(std::any)) == 0;
			}

		}
	}
}