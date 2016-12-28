#include <iostream>
#include <functional>


double square(double z) { return z * z; }


double sum(double x) {	return x; }

template<class... Args>
double sum(double x, Args&&... args) {
	return x + sum(args...);
}

template<class F>
double sum1(const F &f, double x) {	return f(x); }

template<class F, class... Args>
double sum1(const F &f, double x, Args&&... args) {
	return f(x) + sum1(f, args...);
}

template<class... Args>
double sums_of_square(Args&&... args) {
	return sum( square(args)... );
}

template<class Func, class... Args>
double sums_of_applies(const Func &&fun, Args&&... args) {
	return sum( fun(args)... );
}


int main()
{
	using namespace std;
	using namespace std::placeholders;

	cout << "sum  is " << sum(1., 2., 3., 4.) << "\n";
	cout << "sum1 is " << sum1(std::bind(square, _1), 1., 2., 3., 4.) << "\n";
	cout << "sum2 is " << sums_of_square( 1., 2., 3., 4.) << "\n";
	cout << "sum3 is " << sums_of_applies(std::bind(square, _1), 1., 2., 3., 4.) << "\n";

	return 0;
}
