#include "stdafx.h"

#include <numeric>
#include <set>
#include <string>
#include <vector>

#include "utils/utils.h"
#include "utils/utils_inl.h"

/* A palindromic number reads the same both ways. The largest palindrome made from the product of two 2-digit numbers is 9009 = 91 × 99.

Find the largest palindrome made from the product of two 3-digit numbers.*/

namespace {

template<typename TFactors, typename TProducts>
void CalcProducts(const TFactors& factors, TProducts& products)
{
    for (const auto outer : factors)
    {
        for (const auto inner : factors)
        {
            products.insert(outer * inner);
        }
    }
}

template <typename T, typename TProducts>
T FindLargestPalindrome(const TProducts& products)
{
    std::string str;
    bool bIsPalindrome = false;
    TProducts::const_reverse_iterator iter = products.rbegin();

    for (; iter != products.rend() && !bIsPalindrome; ++iter)
    {
        str = std::to_string(*iter);
        bIsPalindrome = std::equal(str.begin(), str.begin() + str.size() / 2, str.rbegin());
    }

    return (bIsPalindrome) ? *prev(iter) : 0;
}

template <typename T>
T Simple(int iDigits)
{
    std::vector<T> factors(static_cast<T>(pow(10, iDigits) - pow(10, iDigits - 1)));
    std::iota(factors.begin(), factors.end(), static_cast<T>(pow(10, iDigits - 1)));

    std::set<T> products;

    CalcProducts(factors, products);

    return FindLargestPalindrome<T, std::set<T>>(products);
}

}  // namespace

int main()
{
    Profile([]() { Print(Simple<int>(2)); });
    Profile([]() { Print(Simple<int>(3)); });

    return 0;
}

