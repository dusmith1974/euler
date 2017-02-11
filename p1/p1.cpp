// p1.cpp : Defines the entry point for the console application.
//

// dsmith1974 P@ssword

#include "stdafx.h"

#include <ctime>

#include <chrono>
#include <functional>
#include <iostream>
#include <numeric>
#include <vector>

/* If we list all the natural numbers below 10 that are multiples of 3 or 5, we get 3, 5, 6 and 9. The sum of these multiples is 23.
   Find the sum of all the multiples of 3 or 5 below 1000. */

void ShowResult(int iResult)
{
    std::cout << "Sum is " << iResult; // << std::endl;
}

int Simple(int iMax)
{
    std::vector<int> vec;

    for (int i = 0; i < iMax; ++i)
    {
        if (i % 3 == 0 || i % 5 == 0)
        {
            vec.push_back(i);
        }
    }

    return std::accumulate(vec.begin(), vec.end(), 0);
}

int SumDivisibleBy(int iBy, int iMax)
{
    int p = (iMax - 1) / iBy;

    return iBy * (p * (p + 1) / 2);
}

int Optimised(int iMax)
{
    return SumDivisibleBy(3, iMax) + SumDivisibleBy(5, iMax) - SumDivisibleBy(3 * 5, iMax);
}

void Profile(std::function<void()> func)
{
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    func();

    end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);

    const int nSize = 26;
    char buf[nSize];

    ctime_s(buf, nSize, &end_time);

    std::cout /*<< "finished computation at " << buf*/ << " in " << elapsed_seconds.count() << "s\n";
}

int main()
{
    Profile([]() { ShowResult(Simple(10)); });
    Profile([]() { ShowResult(Simple(1000)); }); // answer is 233168
    Profile([]() { ShowResult(Simple(1000000)); });

    Profile([]() { ShowResult(Optimised(10)); });
    Profile([]() { ShowResult(Optimised(1000)); });
    Profile([]() { ShowResult(Optimised(1000000)); });

    return 0;
}

