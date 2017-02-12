// p4.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "utils/utils.h"
#include "utils/utils_inl.h"

/* Problem description.*/

template <typename T>
T Simple()
{
    return 0;
}

int main()
{
    Profile([]() { Print(Simple<int>()); });

    return 0;
}

