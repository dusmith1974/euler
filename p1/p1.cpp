// p1.cpp : Defines the entry point for the console application.
//

// dsmith1974 P@ssword

#include "stdafx.h"

#include <numeric>
#include <vector>

#include "utils/utils.h"
#include "utils/utils_inl.h"

/* If we list all the natural numbers below 10 that are multiples of 3 or 5, we get 3, 5, 6 and 9. The sum of these multiples is 23.
   Find the sum of all the multiples of 3 or 5 below 1000. */

template<typename T>
T SumDivisibleBy(T by, T maxVal)
{
    T p = (maxVal - 1) / by;

    return by * (p * (p + 1) / 2);
}

template <typename T>
T Simple(T maxVal)
{
    std::vector<T> vec;

    for (T i = 0; i < maxVal; ++i)
    {
        if (i % 3 == 0 || i % 5 == 0)
        {
            vec.push_back(i);
        }
    }

    return std::accumulate(vec.begin(), vec.end(), 0);
}

template<typename T>
int Optimised(T maxVal)
{
    return SumDivisibleBy(3, maxVal) + SumDivisibleBy(5, maxVal) - SumDivisibleBy(3 * 5, maxVal);
}

int main()
{
    Profile([]() { Print(Simple(10)); });
    Profile([]() { Print(Simple(1000)); }); // answer is 233168
    Profile([]() { Print(Simple(1000000)); });

    Profile([]() { Print(Optimised(10)); });
    Profile([]() { Print(Optimised(1000)); });
    Profile([]() { Print(Optimised(1000000)); });

    return 0;
}

