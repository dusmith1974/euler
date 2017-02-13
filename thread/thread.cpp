// thread.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <assert.h>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

void fn()
{
    for (int i = 0; i < 5; ++i)
        std::cout << "the quick brown fox jumped over the lazy cow." << std::endl;
}

void fn2(std::string& str)
{
    for (int i = 0; i < 5; ++i)
    {
        std::cout << str << std::endl;
        str += std::to_string(std::rand());
    }
}

struct Widget
{
    Widget(int iState)
    {
        m_iState = iState;
    }

    int m_iState;
};

void fn4(std::unique_ptr<Widget> pWidget)
{
    std::cout << "Widget state is " << pWidget->m_iState << std::endl;
}

class A
{
public:

    void show(std::string str)
    {
        std::cout << "A::show() - " << str << std::endl;
    }
};

// RAII - provides exception safety and lets call site continue with other work before scope exit joins thread.
class thread_guard
{
public:
    explicit thread_guard(std::thread& t_) : t(t_) 
    {
    }

    ~thread_guard()
    {
        if (t.joinable())
        {
            t.join();
        }
    }

    thread_guard(thread_guard const&) = delete;
    thread_guard& operator=(thread_guard const&) = delete;

private:
    std::thread& t;
};

int main()
{
    std::string str = "string passed as param";

    std::thread t1(fn);
    std::thread t2(fn2, std::ref(str)); // use ref for &params (if want result back) : never use literal string "..." use std::string("...") 

    t1.join();
    t2.join();

    A a;
    std::thread t3(&A::show, &a, std::string("convert char* to std::string before thread is underway")); // mem fn()
    t3.join();

    std::unique_ptr<Widget> pWidget = std::make_unique<Widget>(5);
    std::thread t4(fn4, std::move(pWidget)); // named movable objects must be explicitly moved.
    t4.join();

    { // RAII
        std::thread t5(fn2, std::string("***T5***\n"));
        thread_guard g(t5);

        std::cout << "other business at the call site\n";
    }

    // take care passing references/pointers to threads - the data always needs to outlive the thread.

    std::thread t6(fn2, std::string("!!detached thread!!"));
    t6.detach(); // can only call detach when joinable is true
    assert(!t6.joinable()); // can't wait for a detached thread - now handled by CRT 
      // suitable for fire and forget or daemon threads (file system monitors etc that run the whole time)

    // MDI eg Word - fnX() may spawn and detach another unrelated fnX()

    // transferring ownership
    //void some_function();
    //void some_other_function();
    //std::thread t1(some_function);
    //std::thread t2 = std::move(t1);
    //t1 = std::thread(some_other_function); // temps are auto and implicit
    //std::thread t3;
    //t3 = std::move(t2);
    //t1 = std::move(t3); // would call terminate as t1 is still running

    /*std::thread f() // out of function
    {
        void some_function();
        return std::thread(some_function);
    }*/

    //void f(std::thread t); // into function
    //void g()
    //{
    //    void some_function();
    //    f(std::thread(some_function));
    //    std::thread t(some_function);
    //    f(std::move(t));
    //}
    

    



    return 0;
}

