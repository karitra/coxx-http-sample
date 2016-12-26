#include <iostream>
#include <cstdlib>

#include <boost/mpl.h>

using namespace std;

struct test_t {
	typedef boost::mpl::list<int,string,string>::type arg_type;
};


int main(int argc, char *argv[]) {

	return EXIT_SUCCESS;
}
