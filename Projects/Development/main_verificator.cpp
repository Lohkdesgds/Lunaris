#include <Lunaris/utility.h>

#include <string>
#include <thread>

using namespace Lunaris;

/*enum class properties_ex_0 { PROP_0, PROP_1, PROP_2, _SIZE };


const std::initializer_list<multi_pair<float, properties_ex_0, char, int>> properties_ex_default = {
	{34.622f,	properties_ex_0::PROP_0, 'a',  5},
	{437.7f,	properties_ex_0::PROP_1, 'b', 10},
	{8864.669f,	properties_ex_0::PROP_2, 'c', 15}
};

class example : 
	public __multi_map_work<multi_map, int, char, int>, 
	public __fixed_multi_map_work<fixed_multi_map, static_cast<size_t>(properties_ex_0::_SIZE), float, properties_ex_0, char, int> {
public:
	example() :
		__multi_map_work<multi_map, int, char, int>({ {99, 'a', 1}, {198, 'b', 2}, {297, 'c', 3} }),
		__fixed_multi_map_work<fixed_multi_map, static_cast<size_t>(properties_ex_0::_SIZE), float, properties_ex_0, char, int>(properties_ex_default)
	{}
};*/


void thread_launch(bool& bad, const std::string& text)
{
	while (!bad) {
		cout << console::color::RED << text << console::color::GOLD << " -> " << console::color::GREEN << rand() % 1000 << console::color::BLUE << " so good";
		std::this_thread::sleep_for(std::chrono::microseconds(10));
	}
}

int already_testing();

int main() {
	/*example yoo;

	cout << "many_types at int 1 has int value of = " << yoo.get<int>(1);
	cout << "many_types at int 2 has int value of = " << yoo.get<int>(2);
	cout << "many_types at int 3 has int value of = " << yoo.get<int>(3);

	cout << "many_types at int 1 has float value of = " << yoo.get<float>(5);
	cout << "many_types at int 2 has float value of = " << yoo.get<float>(10);
	cout << "many_types at int 3 has float value of = " << yoo.get<float>(15);

	yoo.get<int>(1) = 10;

	cout << "many_types at int 1 now has int value of = " << yoo.get<int>(1);*/

	already_testing();
}

int already_testing()
{
	cout << "Hello world!" << " This is some testing, like 32 = " << 32 << " right?";
	cout << "I mean, this is crazy, right?";
	cout << "Imagine a world where you can " << console::color::AQUA << "change colors as you please!";
	cout << "Pretty cool, huh?";

	std::this_thread::sleep_for(std::chrono::seconds(1));

	bool die = false;

	std::thread thrs[] = { 
		std::thread([&] {thread_launch(die, "POTATO THREAD"); }),
		std::thread([&] {thread_launch(die, "SAUCE THREAD"); }),
		std::thread([&] {thread_launch(die, "PICKLE THREAD"); }),
		std::thread([&] {thread_launch(die, "BARBECUE THREAD"); }),
		std::thread([&] {thread_launch(die, "VERYSUS THREAD"); })
	};

	std::this_thread::sleep_for(std::chrono::seconds(1));

	die = true;
	for (auto& i : thrs) i.join();


	promise<int> todo;

	auto fut_test = todo.get_future()
		.then([](int a) {cout << "INT=" << a; return 3.14f * static_cast<float>(a); })
		.then([](float a) {cout << "FLOAT=" << a; })
		.then([] {cout << "The end!"; })
		.then([] {cout << "Well done huh?"; });

	todo.set_value(432);

	fut_test.get();

	multiple_data<int, char, double, float> supadupa;

	supadupa = 3.14f;
	supadupa = 15.54978;
	supadupa = 'a';
	supadupa = 15;

	cout << "Set? " << static_cast<float>(supadupa);
	cout << "Set? " << static_cast<int>(supadupa);
	cout << "Set? " << static_cast<double>(supadupa);
	cout << "Set? " << static_cast<char>(supadupa);

	multiple_data<int, char, double, float> othar = supadupa;

	cout << "One equal? " << (othar == supadupa ? "YES" : "NO");
	cout << "All equal? " << (othar.is_all_equal(supadupa) ? "YES" : "NO");


	multi_map<int, char, unsigned, int, double> mymap;

	//mymap.insert({10, 'a', 15u, 20, 3.14});

	mymap += {10, 'a', 15u, 20, 3.14};
	mymap += {15, 'b', 4532u, 999, 12.54234};


	cout << "1) Value at 'b': " << mymap['b'];
	cout << "1) Value at 4532u: " << mymap[4532u];
	cout << "1) Value at 15u: " << mymap[15u];

	fixed_multi_map<std::string, 3, int, char> fixedmap({
		{ std::string("Stringo"), 10, 'a' },
		{ std::string("Stringa"), 12, 'b' },
		{ std::string("Stringy"), 15, 'c' }
	});

	cout << "2) Value at 'a': " << fixedmap['a'];
	cout << "2) Value at 'b': " << fixedmap['b'];
	cout << "2) Value at 'c': " << fixedmap['c'];

	return 0;
}

