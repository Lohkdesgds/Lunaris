#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

const char ends[] = " \r\n";

int main(int argc, char* argv[]) 
{
	if (argc != 3) {
		std::cout << "Call it <executable> <file_versioning_path> <key> please." << std::endl;
		return 1;
	}

	std::fstream fp;
	fp.open(argv[1], std::ios_base::in | std::ios_base::out | std::ios_base::binary);

	if (!fp.is_open() || !fp.good()) {
		std::cout << "Failed to open '" << argv[1] << "'." << std::endl;
		return 2;
	}

	std::stringstream buffer;
	buffer << fp.rdbuf();
	std::string cpy = buffer.str();

	const std::string& secarg = argv[2];

	size_t pos = cpy.find(secarg);
	if (pos == std::string::npos) {
		std::cout << "I did not find any '" << secarg << "' in the file." << std::endl;
		return 2;
	}
	
	pos += secarg.size() + 1; // NAME + ' '

	unsigned long long reee = 0;
	if (sscanf_s(cpy.data() + pos, "%llu", &reee) != 1) {
		std::cout << "I could not read the number correctly." << std::endl;
		return 3;
	}

	const size_t star = pos; // begin to del
	while (std::find(std::begin(ends), std::end(ends), cpy[pos]) == std::end(ends)) ++pos; // skip anything else?

	cpy.erase(cpy.begin() + star, cpy.begin() + pos);

	cpy.insert(star, std::to_string(reee + 1));

	fp.seekg(0, std::ios_base::beg);

	fp.write(cpy.data(), cpy.size());
	fp.flush();

	return 0;

}