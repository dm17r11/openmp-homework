#include <iostream>
#include <cstdlib>
#include <string>
#include <omp.h>
#include <vector>
using namespace std;

double t;
double sum = 0;
#define T_START t = omp_get_wtime();
#define T_END(S) cout << "Sections " << S << " Time: " << omp_get_wtime() - t << endl;

#define POSLED(CODE) \
sum = 0;\
T_START;\
{\
	for (int i = 0; i < n; i++)\
		{CODE}\
}\
T_END(1)\
cout << sum << "\n";

#define PARALL2(CODE) \
sum = 0;\
T_START;\
__pragma("omp parallel sections reduction(+:sum)")\
{\
__pragma("omp section")\
	{\
		for (int i = 0; i < n/2; i++)\
			{CODE}\
	}\
__pragma("omp section")\
	{\
		for (int i = n/2; i < n; i++)\
			{CODE}\
	}\
}\
T_END(2)\
cout << sum << "\n";

#define PARALL4(CODE) \
sum = 0;\
T_START;\
__pragma("omp parallel sections reduction(+:sum)")\
{\
__pragma("omp section")\
	{\
		for (int i = 0; i < n/4; i++)\
			{CODE}\
	}\
__pragma("omp section")\
	{\
		for (int i = n/4; i < n/2; i++)\
			{CODE}\
	}\
__pragma("omp section")\
	{\
		for (int i = n/2; i < (n*3)/4; i++)\
			{CODE}\
	}\
__pragma("omp section")\
	{\
		for (int i = (n*3)/4; i < n; i++)\
			{CODE}\
	}\
}\
T_END(4)\
cout << sum << "\n";

#define TASK(CODE)\
POSLED(CODE);\
PARALL2(CODE);\
PARALL4(CODE);\

typedef long long int64;

void task1_2(int64 n) {	
	TASK(
		double x = (2. * i - 1.) / (2. * n);
		sum += 4 * 1 / (1 + x * x) / n;
	);
}

bool task1_6(int64 n, const char* s) {
	int sum1 = 0;
	int sum2 = 0;
	int sum3 = 0;
	int min1 = 0;
	int min2 = 0;
	int min3 = 0;

#pragma omp parallel sections
	{
#pragma omp section
		{
			for (int i = 0; i < n/3; i++) 
			{
				sum1 += s[i] == '(' ? 1 : -1;
				if (sum1 < min1)
					min1 = sum1;
			}
		}
#pragma omp section
		{
			for (int i = n/3; i < (n*2)/3; i++)
			{
				sum2 += s[i] == '(' ? 1 : -1;
				if (sum2 < min2)
					min2 = sum2;
			}
		}
#pragma omp section
		{
			for (int i = (n * 2) / 3; i < n; i++)
			{
				sum3 += s[i] == '(' ? 1 : -1;
				if (sum3 < min3)
					min3 = sum3;
			}
		}
	}

	if (min1 < 0)
		return false;
	if (min2 + sum1 < 0)
		return false;
	if (min3 + sum1 + sum2 < 0)
		return false;
	if (sum1 + sum2 + sum3 != 0)
		return false;


	//cout << "Time: " << omp_get_wtime() - t << endl;
	return true;
}

typedef double d2d(double);

namespace task2_1
{
	double f(double x)
	{
		return 1/(1+x*x);
	}

	double integr(double a, double b, d2d func)
	{
		double sum = 0;
		for (int i = 1; i <= 10; ++i)
		{
			//cout << (a + (b - a) * (2 * i - 1) / (2 * 10.)) << "\n";
			sum += func(a + (b-a) * (2*i - 1)/(2*10.));
		}
		return sum / 10. * (b-a);
	}

	double integral(double a, double b, d2d func)
	{
		double whole = integr(a, b, func);
		double left = integr(a, (a+b)/2., func);
		double right = integr((a + b) / 2., b, func);

		if (abs(left + right - whole) < 1e-10)
		{
			return whole;
		}
		else
		{
			cout << "WRONG " << a << " " << b << " err= " << abs(left + right - whole) << "\n";
			cout << left << " " << right << " " << whole << "\n";

			#pragma omp task shared(left)
				left = integral(a, (a + b) / 2., func);
			#pragma omp task shared(right)
				right = integral((a + b) / 2., b, func); 
			#pragma omp taskwait

			return left + right;
		}
	}

	template<class RandomIt>
	void quick_sort(RandomIt first, RandomIt end) {
		auto pivot = *first;
		auto const middle1 = std::partition(first, last, [=](auto const& elem) {
			return elem < pivot;
		});
		auto const middle2 = std::partition(middle1, last, [=](auto const& elem) {
			return pivot >= elem;
		});
		quick_sort(first, middle1);
		quick_sort(middle2, last);
	}
}

int main()
{
	/*task1_2(1e5);*/

	//std::string s = "()()()()(())()()";
	//cout << ((task1_6(s.size(), s.c_str())) ? "correct\n" : "wrong\n");

	#pragma omp parallel
	{
		#pragma omp single
		{
			//cout << task2_1::integral(0, 5, task2_1::f) << "\n";

			std::vector<int> vec;
			vec.push_back(5);
			vec.push_back(2);
			vec.push_back(17);
			vec.push_back(9);
			vec.push_back(0);
			vec.push_back(4);

			task2_1::quick_sort(vec.begin(), vec.end());

			for (auto e : vec)
			{
				cout << e << "\n";
			}
		}
	}
}