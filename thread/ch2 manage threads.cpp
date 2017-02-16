//This chapter covers
//Starting threads, and various ways of specifying
//code to run on a new thread
//Waiting for a thread to finish versus leaving it
//to run
//Uniquely identifying threads

#include "stdafx.h"

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

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

// Improvement on thread_guard - takes ownership so don't have to worry about guard outliving thread
// and prevents joins or detaches by others
class scoped_thread
{
private:
    std::thread t;

public:
    explicit scoped_thread(std::thread t_) : t(std::move(t_))
    {
        if (!t.joinable())
            throw std::logic_error("No thread");
    }

    ~scoped_thread()
    {
        t.join();
    }

    scoped_thread(scoped_thread const&) = delete;
    scoped_thread& operator=(scoped_thread const&) = delete;
};

// call like
// scoped_thread t(std::thread(func(some_local_state)));

// (naive) parallel accumulate
// hardware_concurrency = #cores (hint) may be 0
// doesn't handle exceptions - see ch8
template<typename Iterator, typename T>
struct accumulate_block
{
    void operator()(Iterator first, Iterator last, T& result)
    {
        result = std::accumulate(first, last, result);
    }
};

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init)
{
    unsigned long const length = std::distance(first, last);

    if (!length)
        return init;

    unsigned long const min_per_thread = 25;
    unsigned long const max_threads =
        (length + min_per_thread - 1) / min_per_thread;
    unsigned long const hardware_threads =
        std::thread::hardware_concurrency();
    unsigned long const num_threads =
        std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length / num_threads;

    std::vector<T> results(num_threads);
    std::vector<std::thread> threads(num_threads - 1);
    Iterator block_start = first;

    for (unsigned long i = 0; i < (num_threads - 1); ++i)
    {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        threads[i] = std::thread(
            accumulate_block<Iterator, T>(),
            block_start, block_end, std::ref(results[i]));
        block_start = block_end;
    }
    accumulate_block<Iterator, T>()(
        block_start, last, results[num_threads - 1]);

    std::for_each(threads.begin(), threads.end(),
        std::mem_fn(&std::thread::join));

    return std::accumulate(results.begin(), results.end(), init);
}

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
    
    // subdivide work of an algorithm by spawning multiple threads
    // step towards a thread group/automated management
    // doesn't handle return values - see ch4
    std::vector<std::thread> threads;
    for (unsigned i = 0; i < 5; ++i)
    {
        threads.push_back(std::thread(fn2, std::string("multiple threads.\n")));
    }
    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
    
    // thread ids
    // t.get_id() or this_thread::get_ifd()
    // returns id of running thread or not any thread
    // can be used for storing in maps too
    // used to specialize algorithms, eg if this_thread.get_id() == master_thread then do something extra



    return 0;
}

