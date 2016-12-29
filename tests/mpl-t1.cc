#include <iostream>
#include <cstdlib>

#include <boost/mpl/list.hpp>
#include <boost/mpl/for_each.hpp>

using namespace std;

namespace mpl = boost::mpl;

struct test_t {
	typedef mpl::list<int, string, string>::type arg_type;
};

struct Printer {


	

	void operator()(const int v) {
		cout << "boo(int): " << v << '\n';
	}

	void operator()(const string &s) {
		cout << "boo(str): " << s << '\n';
	}


};

int main(int argc, char *argv[]) {

	cout << "running `for_each` algo...\n";

	test_t::arg_type packet; // (100, "hello", "mpl");
	Printer p;
	mpl::for_each<test_t::arg_type>(p);

	return EXIT_SUCCESS;
}
