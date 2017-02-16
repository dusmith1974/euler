//This chapter covers
//Problems with sharing data between threads
//Protecting data with mutexes
//Alternative facilities for protecting shared data

#include <list>
#include <mutex>
#include <algorithm>

std::list<int> some_list;
std::mutex some_mutex;

// guard list with mutex
void add_to_list(int new_value)
{
    std::lock_guard<std::mutex> guard(some_mutex);
    some_list.push_back(new_value);
}

bool list_contains(int value_to_find)
{
    std::lock_guard<std::mutex> guard(some_mutex);

    return std::find(some_list.begin(), some_list.end(), value_to_find)
        != some_list.end();
}

//Beware backdoors to circumvent the above - Don’t pass pointers and references to protected data outside the scope of the lock, whether by
//returning them from a function, storing them in externally visible memory, or passing them as
//arguments to user - supplied functions
//safer to return copies

// call sequence on stack, !empty() pop() no longer safe
// pop() could throw, but now empty is redundant and caller needs to handle exceptions
 // could pass in a ref to receive pop, could only use no-throw ctor objects, best yet just return a shared_ptr from pop

// thread safe stack
#include <exception>
#include <memory>
#include <mutex>
#include <stack>

struct empty_stack : std::exception
{
    const char* what() const throw();
};

template<typename T>
class threadsafe_stack
{
private:
    std::stack<T> data;
    mutable std::mutex m;
public:
    threadsafe_stack() {}
    threadsafe_stack(const threadsafe_stack& other)
    {
        std::lock_guard<std::mutex> lock(other.m);
        data = other.data;
    }
    threadsafe_stack& operator=(const threadsafe_stack&) = delete;
    void push(T new_value)
    {
        std::lock_guard<std::mutex> lock(m);
        data.push(new_value);
    }
    std::shared_ptr<T> pop()
    {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();
        std::shared_ptr<T> const res(std::make_shared<T>(data.top()));
        data.pop();
        return res;
    }
    void pop(T& value)
    {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();
        value = data.top();
        data.pop();
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};

// deadlock
// 2 or more mutexes (drum, drumstick)
// always lock in the same order (works for most cases but may fail if locking two objects of the same type) lhs, rhs (2 threads reverse order)

//  std::lock can lock 2 mutexes w/o deadlock
class some_big_object;
void swap(some_big_object& lhs, some_big_object& rhs) {};

class X
{
private:
    some_big_object some_detail;
    std::mutex m;

public:
    X(some_big_object const& sd) : some_detail(sd) {}

    friend void swap(X& lhs, X& rhs)
    {
        if (&lhs == &rhs) // re-lock throws (unless recursive mutex)
            return;

        std::lock(lhs.m, rhs.m); // lock both mutexes
        std::lock_guard<std::mutex> lock_a(lhs.m, std::adopt_lock); // inherit locked mutexes
        std::lock_guard<std::mutex> lock_b(rhs.m, std::adopt_lock);

        swap(lhs.some_detail, rhs.some_detail);
    }
};

// deadlock can also occur with two join()s
// don't wait for a thread if there's a chance it's waiting for you
// avoid calling user supplied code while holding a lock
// acquire locks in a fixed order
// hand over hand locking - order of traversal, disallow reverse
// lock hierarchy - div app into layers, id mutexes that can be locked in each layer, don't lock mutex if already hold lock from lower layer

hierarchical_mutex high_level_mutex(10000);
hierarchical_mutex low_level_mutex(5000);
int do_low_level_stuff();
int low_level_func()
{
    std::lock_guard<hierarchical_mutex> lk(low_level_mutex);
    return do_low_level_stuff();
}
void high_level_stuff(int some_param);
void high_level_func()
{
    std::lock_guard<hierarchical_mutex> lk(high_level_mutex);
    high_level_stuff(low_level_func());
}
void thread_a()
{
    high_level_func();
}
hierarchical_mutex other_mutex(100);
void do_other_stuff();
void other_stuff()
{
    high_level_func();
    do_other_stuff();
}
void thread_b()
{
    std::lock_guard<hierarchical_mutex> lk(other_mutex);
    other_stuff();
}

class hierarchical_mutex
{
    std::mutex internal_mutex;
    unsigned long const hierarchy_value;
    unsigned long previous_hierarchy_value;
    static thread_local unsigned long this_thread_hierarchy_value;
    void check_for_hierarchy_violation()
    {
        if (this_thread_hierarchy_value <= hierarchy_value)
        {
            throw std::logic_error(“mutex hierarchy violated”);
        }
    }
    void update_hierarchy_value()
    {
        previous_hierarchy_value = this_thread_hierarchy_value;
        this_thread_hierarchy_value = hierarchy_value;
    }
public:
    explicit hierarchical_mutex(unsigned long value) :
        hierarchy_value(value),
        previous_hierarchy_value(0)
    {}
    
    void lock()
    {
        check_for_hierarchy_violation();
        internal_mutex.lock();
        update_hierarchy_value();
    }
    void unlock()
    {
        this_thread_hierarchy_value = previous_hierarchy_value;
        internal_mutex.unlock();
    }
    bool try_lock()
    {
        check_for_hierarchy_violation();
        if (!internal_mutex.try_lock())
            return false;
        update_hierarchy_value();
        return true;
    }
};

thread_local unsigned long
hierarchical_mutex::this_thread_hierarchy_value(ULONG_MAX);
// flexible locking with unique_lock
// unique_lock can be ctor with defer_lock
// same code as above, unique_lock is slightly larger and slower

class some_big_object;
void swap(some_big_object& lhs, some_big_object& rhs);

class X
{
private:
    some_big_object some_detail;
    std::mutex m;
public:
    X(some_big_object const& sd) :some_detail(sd) {}
    friend void swap(X& lhs, X& rhs)
    {
        if (&lhs == &rhs)
            return;
        std::unique_lock<std::mutex> lock_a(lhs.m, std::defer_lock);
        std::unique_lock<std::mutex> lock_b(rhs.m, std::defer_lock);
        std::lock(lock_a, lock_b);
        swap(lhs.some_detail, rhs.some_detail);
    }
};

// transferring mutex ownership between scopes
// ownership xfer is auto for rvalues and explicit (move()) for lvalue (to avoid accidentally xfer ownership from a variable)

std::unique_lock<std::mutex> get_lock()
{
    extern std::mutex some_mutex;
    std::unique_lock<std::mutex> lk(some_mutex);
    prepare_data();
    return lk;
}
void process_data()
{
    std::unique_lock<std::mutex> lk(get_lock());
    do_something();
}

// fine grained, course grained locks
// course - supermarket checkout
// avoid file io
// unique_locks can be unlocked

void get_and_process_data()
{
    std::unique_lock<std::mutex> my_lock(the_mutex);
    some_class data_to_process = get_next_data_chunk();
    my_lock.unlock();
    result_type result = process(data_to_process);
    my_lock.lock();
    write_result(data_to_process, result);
}

// locking one mutex at a time in op==

class Y
{
private:
    int some_detail;
    mutable std::mutex m;
    int get_detail() const
    {
        std::lock_guard<std::mutex> lock_a(m);
        return some_detail;
    }
public:
    Y(int sd) :some_detail(sd) {}
    friend bool operator==(Y const& lhs, Y const& rhs)
    {
        if (&lhs == &rhs)
            return true;
        int const lhs_value = lhs.get_detail();
        int const rhs_value = rhs.get_detail();
        return lhs_value == rhs_value;
    }
};

// protecting shared data during initialization
// (some data only needs protected on ctor, maybe r/o or lock free)
// single threaded lazy initialization

std::shared_ptr<some_resource> resource_ptr;
void foo()
{
    if (!resource_ptr)
    {
        resource_ptr.reset(new some_resource);
    }

    resource_ptr->do_something();
}

// avoid the double-checked locking pattern (leads to race conds)
// instead use once_flag and call_once

std::shared_ptr<some_resource> resource_ptr;
std::once_flag resource_flag;
void init_resource()
{
    resource_ptr.reset(new some_resource);
}
void foo()
{
    std::call_once(resource_flag, init_resource);
    resource_ptr->do_something();
}

class X
{
private:
    connection_info connection_details;
    connection_handle connection;
    std::once_flag connection_init_flag;
    void open_connection()
    {
        connection = connection_manager.open(connection_details);
    }
public:
        X(connection_info const& connection_details_) :
        connection_details(connection_details_)
    {}
    void send_data(data_packet const& data)
    {
        std::call_once(connection_init_flag, &X::open_connection, this);
        connection.send_data(data);
    }
    data_packet receive_data()
    {
        std::call_once(connection_init_flag, &X::open_connection, this);
        return connection.receive_data();
    }
};

// protecting rarely updated data structures eg DNS records
// single (exclusive) writer multiple readers (proposed to std but not accepted so use boost)

#include <map>
#include <string>
#include <mutex>
#include <boost/thread/shared_mutex.hpp>
class dns_entry;
class dns_cache
{
    std::map<std::string, dns_entry> entries;
    mutable boost::shared_mutex entry_mutex;
public:
    dns_entry find_entry(std::string const& domain) const
    {
        boost::shared_lock<boost::shared_mutex> lk(entry_mutex);
        std::map<std::string, dns_entry>::const_iterator const it =
            entries.find(domain);
        return (it == entries.end()) ? dns_entry() : it->second;
    }
    void update_or_add_entry(std::string const& domain,
        dns_entry const& dns_details)
    {
        std::lock_guard<boost::shared_mutex> lk(entry_mutex);
        entries[domain] = dns_details;
    }
};

// recursive locking
// recursive_mutex()
// lock() * n, unlock * n (in same thread)
// don't overuse - may be result of poor design
// possible use, many member functions
