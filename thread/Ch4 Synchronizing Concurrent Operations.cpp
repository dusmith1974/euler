//Waiting for an event
//Waiting for one - off events with futures
//Waiting with a time limit
//Using synchronization of operations to
//simplify code

// synchronize operations between threads via condition variables or futures

#include "stdafx.h"

#include <thread>
#include <mutex>


// ok but sleep is too little or too much
bool flag;
std::mutex m;
void wait_for_flag()
{
    std::unique_lock<std::mutex> lk(m);
    while (!flag)
    {
        lk.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        lk.lock();
    }
}

// condition_variable // req mutex
// condition_variable_any // or mutex like // additional cost

std::mutex mut;
std::queue<data_chunk> data_queue;
std::condition_variable data_cond;

void data_preparation_thread()
{
    while (more_data_to_prepare())
    {
        data_chunk const data = prepare_data();
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(data);
        data_cond.notify_one(); // notifies one random waiting thread. notify_all() notifies all waiting threads.
    }
}

void data_processing_thread()
{
    while (true)
    {
        std::unique_lock<std::mutex> lk(mut);

        data_cond.wait(lk, [] {return !data_queue.empty(); }); // wait for non-empty q
         // if q.empty(), unlock lk and block/sleep until notify_one(), re-aquire lock and re-check
         // don't use side effects in condition
         // loop/condition protects against spurious wakeup

        data_chunk data = data_queue.front();
        data_queue.pop();
        lk.unlock();
        process(data);

        if (is_last_chunk(data))
            break;
    }
}

// FULL cv queue
template<typename T>
class threadsafe_queue
{
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    threadsafe_queue()
    {}
    threadsafe_queue(threadsafe_queue const& other)
    {
        std::lock_guard<std::mutex> lk(other.mut);
        data_queue = other.data_queue;
    }
    void push(T new_value)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }
    void wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] {return !data_queue.empty(); });
        value = data_queue.front();
        data_queue.pop();
    }
    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] {return !data_queue.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }

    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return false;
        value = data_queue.front();
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};

// use futures for one off events - like an airplane call
#include <future>
#include <iostream>
int find_the_answer_to_ltuae();
void do_other_stuff();
int main()
{
    std::future<int> the_answer = std::async(find_the_answer_to_ltuae);
    do_other_stuff();
    std::cout << "The answer is " << the_answer.get() << std::endl;
}

auto f6 = std::async(std::launch::async, Y(), 1.2);
auto f7 = std::async(std::launch::deferred, baz, std::ref(x));
auto f8 = std::async(
    std::launch::deferred | std::launch::async, // the default - implementation chooses
    baz, std::ref(x));
auto f9 = std::async(baz, std::ref(x));
f7.wait();

// can use packaged_task<> or promise<> instead of async()
// packaged_task<> is the higher level abstraction

// packaged_task ties future to callable object op() return value -> future
// good for task management schemes eg thread pool, task per thread, or sequential tasks on background thread

// std::packaged_task <std::string(std::vector<char>*, int)> // future<string>, op() is (vector<char>*, int)>

std::mutex m;
std::deque<std::packaged_task<void()> > tasks;
bool gui_shutdown_message_received();
void get_and_process_gui_message();

void gui_thread()
{
    while (!gui_shutdown_message_received()) // handle msgs and posted tasks
    {
        get_and_process_gui_message();
        std::packaged_task<void()> task;
        {
            std::lock_guard<std::mutex> lk(m);
            if (tasks.empty())
                continue;
            task = std::move(tasks.front()); // extract task from q
            tasks.pop_front();
        }
        task(); // and run it, future made ready on completion
    }
}

std::thread gui_bg_thread(gui_thread); // posting a task
template<typename Func>
std::future<void> post_task_for_gui_thread(Func f)
{
    std::packaged_task<void()> task(f); // create packaged task and post on q
    std::future<void> res = task.get_future(); // first extract the future (before it's made ready..)
    std::lock_guard<std::mutex> lk(m);
    tasks.push_back(std::move(task));
    return res; // return future so caller may make use of (or ignore) the result once the task is executed in the above msg loop
}

// if a task can't be expressed as a simple function call, or the result must come from more than one place then use a promise<>
