// p3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <set>

#include "utils/utils.h"
#include "utils/utils_inl.h"

/* The prime factors of 13195 are 5, 7, 13 and 29

What is the largest prime factor of the number 600851475143 ?*/

template<typename T>
T Simple(T n)
{
    T lastFactor = 0;

    if (n % 2 == 0)
    {
        n /= 2;
        lastFactor = 2;

        while (n % 2 == 0)
        {
            n /= 2;
        }
    }
    else
    {
        lastFactor = 1;
    }

    T factor = 3;

    while (n > 1)
    {
        if (n % factor == 0)
        {
            n /= factor;
            lastFactor = factor;

            while (n % factor == 0)
            {
                n /= factor;
            }
        }

        factor += 2;
    }

    return lastFactor;
}

int main()
{
    Profile([]() {Print(Simple(13195)); });
    Profile([]() {Print(Simple(600851475143)); });

    return 0;
}

