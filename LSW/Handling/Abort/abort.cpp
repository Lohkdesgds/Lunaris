#include "abort.h"

namespace LSW {
	namespace v5 {
		namespace Handling {


			Abort::Abort(const std::string& fw, const std::string& dt, const abort::abort_level lv)
				: std::exception(("From: " + fw + ";\nDetails: " + dt + ";\nLevel: " + std::to_string(static_cast<int>(lv))).c_str(), static_cast<int>(lv))
			{
				s_from_where = fw;
				s_details = dt;
				abort_level = lv;
			}
			Abort::Abort(const Abort& abor)
				: std::exception(("From: " + abor.s_from_where + ";\nDetails: " + abor.s_details + ";\nLevel: " + std::to_string(static_cast<int>(abor.abort_level))).c_str(), static_cast<int>(abor.abort_level))
			{
				s_from_where = abor.s_from_where;
				s_details = abor.s_details;
				abort_level = abor.abort_level;
			}
			const std::string& Abort::get_from() const
			{
				return s_from_where;
			}
			const std::string& Abort::get_details() const
			{
				return s_details;
			}
			const abort::abort_level& Abort::get_level() const
			{
				return abort_level;
			}

			Abort::operator std::string() const
			{
				std::string lvl;
				switch (get_level()) {
				case abort::abort_level::FATAL_ERROR:
					lvl = "FATAL_ERROR";
					break;
				case abort::abort_level::GIVEUP:
					lvl = "GIVEUP";
					break;
				case abort::abort_level::OTHER:
					lvl = "OTHER";
					break;
				}
				return "From: " + get_from() + ";\nDetails: " + get_details() + ";\nLevel: " + lvl;
			}

			std::string operator+(const Abort& a) {
				return static_cast<std::string>(a);
			}
		}
	}
}