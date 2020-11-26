#include "superthread.h"

namespace LSW {
	namespace v5 {
		namespace Tools {

			// instantiation (better compile times)

			template SuperThread<void>;
			template SuperThread<bool>;
			template SuperThread<char>;
			template SuperThread<int>;
			template SuperThread<float>;
			template SuperThread<double>;
			template SuperThread<unsigned>;
			template SuperThread<long>;
			template SuperThread<long long>;
			template SuperThread<unsigned long long>;
			template SuperThread<int64_t>;
			template SuperThread<uint64_t>;

		}
	}
}