#include <iostream>
#include <cstdlib>
#include <string>
#include <omp.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
using namespace std;

typedef double d2d(double);


namespace task1
{
    bool brackets_check_a(long long n, const char* s) {
        int sum1 = 0;
        int sum2 = 0;
        int min1 = 0;
        int min2 = 0;

    #pragma omp parallel sections
        {
    #pragma omp section
            {
                for (int i = 0; i < n/2; i++)
                {
                    sum1 += s[i] == '(' ? 1 : -1;
                    if (sum1 < min1)
                        min1 = sum1;
                }
            }
    #pragma omp section
            {
                for (int i = n/2; i < n; i++)
                {
                    sum2 += s[i] == '(' ? 1 : -1;
                    if (sum2 < min2)
                        min2 = sum2;
                }
            }
        }

        if (min1 < 0)
            return false;
        if (min2 + sum1 < 0)
            return false;
        if (sum1 + sum2 != 0)
            return false;

        return true;
    }

    bool brackets_check_b(long long n, const char* s) {
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
}

namespace task2
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
			//cout << "WRONG " << a << " " << b << " err= " << abs(left + right - whole) << "\n";
			//cout << left << " " << right << " " << whole << "\n";

			#pragma omp task shared(left)
				left = integral(a, (a + b) / 2., func);
			#pragma omp task shared(right)
				right = integral((a + b) / 2., b, func);
			#pragma omp taskwait

			return left + right;
		}
	}

	template<class RandomIt>
	void quick_sort(RandomIt first, RandomIt last) {
        if(first == last)
            return;
		auto pivot = *first;
		auto middle1 = std::partition(first, last, [pivot](auto const& elem) {
			return (elem < pivot);
		});
		auto middle2 = std::partition(middle1, last, [pivot](auto const& elem) {
			return !(pivot < elem);
		});
		#pragma omp task
            quick_sort(first, middle1);
		#pragma omp task
            quick_sort(middle2, last);
		#pragma omp taskwait
	}
}

namespace task3
{
    struct Matrix
    {
        double** data;
        int w;
        int h;

        Matrix(int w, int h)
        {
            this->w = w;
            this->h = h;
            data = new double*[h];
            for (int y = 0; y < h; ++y)
                data[y] = new double[w];
        }

        void divide(int y, double k)
        {
            //#pragma omp parallel for
            for (int x = 0; x < w; ++x)
                data[y][x] /= k;
        }

        void subtract(int y1, int y2, double k)
        {
            //#pragma omp parallel for
            for (int x = 0; x < w; ++x)
                data[y2][x] -= data[y1][x] * k;
        }

        void swap_(int y1, int y2)
        {
            for (int x = 0; x < w; ++x)
                swap(data[y1][x], data[y2][x]);
        }

        void print()
        {
            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    cout << data[y][x] << " ";
                }
                cout << "\n";
            }
        }

        void transform_()
        {
            for (int Y = 0; Y < min(h, w); ++Y)
            {
                {
                    int y_of_max = Y;
                    double max_ = abs(data[y_of_max][Y]);
                    for (int y = Y; y < h; ++y)
                    {
                        if (abs(data[y][Y]) > max_)
                        {
                            y_of_max = y;
                            max_ = abs(data[y][Y]);
                        }
                    }
                    swap_(Y, y_of_max);
                    divide(Y, data[Y][Y]);
                }

                #pragma omp parallel for
                for (int y = 0; y < h; ++y)
                {
                    if (y != Y && data[y][Y] != 0)
                        subtract(Y, y, data[y][Y]);
                }
            }
        }
    };
}

int main()
{
    cout << "Task 1 - Bracket sequences\n";
    cout << "Task 2 - Adaptive quadrature and quick sort\n";
    cout << "Task 3 - System of linear equations\n";
    int n = -1;
    while (n < 1 || n > 3)
    {
        cout << "Enter task number (1-3):";
        cin >> n;
    }

    if (n == 1) {
        string str = "()()()()(())()()";
        cout << str << " (2 sections) " << ((task1::brackets_check_a(str.size(), str.c_str())) ? "correct\n" : "wrong\n");
        cout << str << " (3 sections) " << ((task1::brackets_check_b(str.size(), str.c_str())) ? "correct\n" : "wrong\n");

        str = "()()()()(()))))(((()()";
        cout << str << " (2 sections) " << ((task1::brackets_check_a(str.size(), str.c_str())) ? "correct\n" : "wrong\n");
        cout << str << " (3 sections) " << ((task1::brackets_check_b(str.size(), str.c_str())) ? "correct\n" : "wrong\n");
    }
    if (n == 2)
    {
        #pragma omp parallel
        {
            #pragma omp single
            {
                // task 2.1
                cout << task2::integral(0, 5, task2::f) << "\n";

                // task 2.2
                std::vector<int> vec;
                for (int i = 0; i < 25; ++i)
                    vec.push_back(rand()%100);
                for (auto e : vec)
                {
                    cout << e << " ";
                }
                task2::quick_sort(vec.begin(), vec.end());
                cout << "\n";
                for (auto e : vec)
                {
                    cout << e << " ";
                }
            }
        }
    }
    if (n == 3)
    {
        //task 3
        task3::Matrix m(4,3);
        m.data[0][0] = 1; m.data[0][1] = 2; m.data[0][2] = 3; m.data[0][3] = 1;
        m.data[1][0] = 2; m.data[1][1] = 1; m.data[1][2] = 1; m.data[1][3] = 1;
        m.data[2][0] = 7; m.data[2][1] = 0; m.data[2][2] = 1; m.data[2][3] = 1;

        m.print();

        m.transform_();

        cout << "\n";
        m.print();

        int l = 900;
        m = task3::Matrix(l + 1, l);
        for (int y = 0; y < l; ++y)
        {
            for (int x = 0; x < l+1; ++x)
            {
                m.data[y][x] = rand()%1000;
            }
        }

        auto start = std::chrono::high_resolution_clock::now();
        m.transform_();
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

        cout << "900x900 matrix solved in " << microseconds << " microseconds\n";
    }
}
