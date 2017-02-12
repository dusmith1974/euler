// utils.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "utils.h"

#include <chrono>
#include <iostream>

void Profile(std::function<void()> func)
{
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    func();

    end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << " in " << elapsed_seconds.count() << "s\n";
}


