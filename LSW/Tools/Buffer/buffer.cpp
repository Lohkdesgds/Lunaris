#include "buffer.h"

namespace LSW {
	namespace v5 {
		namespace Tools {

			Buffer::Buffer(const std::string& str)
			{
				this->assign(str.begin(), str.end());
			}

			Buffer::Buffer(const char* str)
			{
				std::string _t = str;
				this->assign(_t.begin(), _t.end());
			}

			Buffer::Buffer(const char* ptr, const size_t siz)
			{
				this->assign(ptr, ptr + siz);
			}

			Buffer::Buffer(const void* ptr, const size_t siz)
			{
				this->assign((char*)ptr, (char*)ptr + siz);
			}

			Buffer::operator std::string()
			{
				std::string transf;
				transf.assign(this->begin(), this->end());
				return std::move(transf);
			}

			Buffer::operator const std::string&() const
			{
				_temp.assign(this->begin(), this->end());
				return _temp;
			}

			bool Buffer::operator==(const Buffer& vec) const
			{
				if (vec.size() == this->size()) {
					for (size_t p = 0; p < this->size(); p++) if ((*this)[p] != vec[p]) return false;
					return true;
				}
				return false;
			}

			bool Buffer::operator==(const std::string& str) const
			{
				if (str.size() == this->size()) {
					for (size_t p = 0; p < this->size(); p++) if ((*this)[p] != str[p]) return false;
					return true;
				}
				return false;
			}

			bool Buffer::operator==(const char* str) const
			{
				return strncmp(this->data(), str, this->size()) == 0;
			}

			bool Buffer::operator!=(const Buffer& vec) const
			{
				if (vec.size() == this->size()) {
					for (size_t p = 0; p < this->size(); p++) if ((*this)[p] != vec[p]) return true;
					return false;
				}
				return true;
			}

			bool Buffer::operator!=(const std::string& str) const
			{
				if (str.size() == this->size()) {
					for (size_t p = 0; p < this->size(); p++) if ((*this)[p] != str[p]) return true;
					return false;
				}
				return true;
			}

			bool Buffer::operator!=(const char* str) const
			{
				return strncmp(this->data(), str, this->size()) != 0;
			}

			Buffer& Buffer::operator=(const Buffer& vec)
			{
				this->assign(vec.begin(), vec.end());
				return *this;
			}

			Buffer& Buffer::operator=(const std::string& str)
			{
				this->assign(str.begin(), str.end());
				return *this;
			}

			Buffer& Buffer::operator=(const char* str)
			{
				std::string _t = str;
				this->assign(_t.begin(), _t.end());
				return *this;
			}

			void Buffer::operator+=(const char& ch)
			{
				this->push_back(ch);
			}

			void Buffer::operator+=(const Buffer& vec)
			{
				this->insert(this->end(), vec.begin(), vec.end());
			}

			void Buffer::operator+=(const std::string& str)
			{
				this->insert(this->end(), str.begin(), str.end());
			}

			void Buffer::copy_to(void* ptr, const size_t siz) const
			{
				size_t commsiz = siz;
				if (commsiz >= this->size()) commsiz = this->size();
				std::copy(this->begin(), this->begin() + commsiz, (char*)ptr);
			}

			const std::string Buffer::cast_as_string() const
			{
				_temp.assign(this->begin(), this->end());
				return _temp;
			}
		}
	}
}