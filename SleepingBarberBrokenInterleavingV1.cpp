/*

  ██████  ██▓    ▓█████ ▓█████  ██▓███   ██▓ ███▄    █   ▄████     ▄▄▄▄    ▄▄▄       ██▀███   ▄▄▄▄   ▓█████  ██▀███    ██████
▒██    ▒ ▓██▒    ▓█   ▀ ▓█   ▀ ▓██░  ██▒▓██▒ ██ ▀█   █  ██▒ ▀█▒   ▓█████▄ ▒████▄    ▓██ ▒ ██▒▓█████▄ ▓█   ▀ ▓██ ▒ ██▒▒██    ▒
░ ▓██▄   ▒██░    ▒███   ▒███   ▓██░ ██▓▒▒██▒▓██  ▀█ ██▒▒██░▄▄▄░   ▒██▒ ▄██▒██  ▀█▄  ▓██ ░▄█ ▒▒██▒ ▄██▒███   ▓██ ░▄█ ▒░ ▓██▄
  ▒   ██▒▒██░    ▒▓█  ▄ ▒▓█  ▄ ▒██▄█▓▒ ▒░██░▓██▒  ▐▌██▒░▓█  ██▓   ▒██░█▀  ░██▄▄▄▄██ ▒██▀▀█▄  ▒██░█▀  ▒▓█  ▄ ▒██▀▀█▄    ▒   ██▒
▒██████▒▒░██████▒░▒████▒░▒████▒▒██▒ ░  ░░██░▒██░   ▓██░░▒▓███▀▒   ░▓█  ▀█▓ ▓█   ▓██▒░██▓ ▒██▒░▓█  ▀█▓░▒████▒░██▓ ▒██▒▒██████▒▒
▒ ▒▓▒ ▒ ░░ ▒░▓  ░░░ ▒░ ░░░ ▒░ ░▒▓▒░ ░  ░░▓  ░ ▒░   ▒ ▒  ░▒   ▒    ░▒▓███▀▒ ▒▒   ▓▒█░░ ▒▓ ░▒▓░░▒▓███▀▒░░ ▒░ ░░ ▒▓ ░▒▓░▒ ▒▓▒ ▒ ░
░ ░▒  ░ ░░ ░ ▒  ░ ░ ░  ░ ░ ░  ░░▒ ░      ▒ ░░ ░░   ░ ▒░  ░   ░    ▒░▒   ░   ▒   ▒▒ ░  ░▒ ░ ▒░▒░▒   ░  ░ ░  ░  ░▒ ░ ▒░░ ░▒  ░ ░
░  ░  ░    ░ ░      ░      ░   ░░        ▒ ░   ░   ░ ░ ░ ░   ░     ░    ░   ░   ▒     ░░   ░  ░    ░    ░     ░░   ░ ░  ░  ░
      ░      ░  ░   ░  ░   ░  ░          ░           ░       ░     ░            ░  ░   ░      ░         ░  ░   ░           ░
                                                                        ░                          ░


    Those pesky haircutters are at it again, but this time there's something wrong. Can you help them smooth out their business?
    This is version 1.

    --Seth Kitchen "Lucky for me I have no hair!" sjkyv5@mst.edu February 18, 2019
*/

/***************************************************

                General Includes

****************************************************/
#include <iostream>
#include <thread>        
#include <mutex>         
#include <condition_variable>
#include <queue>
#include <string>

/***************************************************

                Namespaces

****************************************************/
using namespace std;


/***************************************************

                Type Definitions

****************************************************/
/*              Semaphores
    "C/C++ is the most powerful language"
    Oh yeah? If c++ is so great why aren't there built in semaphores?
    Also good luck trying to do a lambda or asyncronous action.
    Shout out to StackOverflow and Github for these:
    https://stackoverflow.com/questions/4792449/c0x-has-no-semaphores-how-to-synchronize-threads
    https://gist.github.com/yohhoy/2156481
*/
class binary_semaphore {
public:
    explicit binary_semaphore(int init_count = count_max)
        : count_(init_count) {}

    // P-operation / acquire
    void wait()
    {
        std::unique_lock<std::mutex> lk(m_);
        cv_.wait(lk, [=] { return 0 < count_; });
        --count_;
    }
    bool try_wait()
    {
        std::lock_guard<std::mutex> lk(m_);
        if (0 < count_) {
            --count_;
            return true;
        }
        else {
            return false;
        }
    }
    // V-operation / release
    void signal()
    {
        std::lock_guard<std::mutex> lk(m_);
        if (count_ < count_max) {
            ++count_;
            cv_.notify_one();
        }
    }

    // Lockable requirements
    void lock() { wait(); }
    bool try_lock() { return try_wait(); }
    void unlock() { signal(); }

private:
    static const int count_max = 1;
    int count_;
    std::mutex m_;
    std::condition_variable cv_;
};

class general_semaphore
{
private:
    std::mutex mutex_;
    std::condition_variable condition_;
    unsigned long count_ = 0; // Initialized as locked.

public:
    void signal() {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        ++count_;
        condition_.notify_one();
    }

    void wait() {
        std::unique_lock<decltype(mutex_)> lock(mutex_);
        while (!count_) // Handle spurious wake-ups.
            condition_.wait(lock);
        --count_;
    }

    bool try_wait() {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        if (count_) {
            --count_;
            return true;
        }
        return false;
    }
};

/***************************************************

                Environment Constants

****************************************************/
/*              Environment
    These can change depending on how big/popular you want your barbershop to be.
*/
const int NUM_BARBERS = 2;
const int NUM_CUSTOMERS = 4;


/***************************************************

                Global Variables

****************************************************/
/*              Environment
    These can change depending on how big/popular you want your barbershop to be.
*/
int numberOfFreeWaitingRoomSeats = 2;

/*              Semaphores                          */
binary_semaphore AccessToWaitingRoomSeats;      // semaphore for if you can put people in seats or take them out of seats: auto-assigned to 1 in constructor
general_semaphore BarberReady, CustomerReady;   // semaphore for if the barber can cut someoneone's hair and if there is a customer who's hair needs to be cut
                                                // auto assigned to 0 in constructor


void GetHairCut(int id)
{
    string s = "The Customer " + to_string(id) + " hears Snip Snip\r\n";
    cout << s;
}

void CutHair(int id)
{
    string s = "The Barber " + to_string(id) + " went Snip Snip\r\n";
    cout << s;
}

/***************************************************

        ONLY MAKE CHANGES BELOW THIS LINE

****************************************************/

void Barber(int thread_num) {
    while (true) {
        CustomerReady.wait();
        AccessToWaitingRoomSeats.wait();
        numberOfFreeWaitingRoomSeats += 1;
        BarberReady.signal();
        AccessToWaitingRoomSeats.signal();
        CutHair(thread_num);
    }
}

void Customer(int thread_num) {
    AccessToWaitingRoomSeats.wait();
    if (numberOfFreeWaitingRoomSeats > 0) {
        numberOfFreeWaitingRoomSeats -= 1;
        CustomerReady.signal();
        AccessToWaitingRoomSeats.signal();
        BarberReady.wait();
        GetHairCut(thread_num);
    }
    else
    {
        //no space, must leave! 
        AccessToWaitingRoomSeats.signal();
    }
}

int main()
{
    int threadnum = NUM_BARBERS + NUM_CUSTOMERS;
    thread threads[NUM_BARBERS + NUM_CUSTOMERS];

    string s = "Running " + to_string(threadnum) + " threads in parallel: \r\n";
    cout << s;

    /* spawn Barber threads */
    for (int id = 0; id < NUM_BARBERS; id++)
        threads[id] = thread(Barber, id);

    /* spawn Customer threads */
    for (int id = 0; id < NUM_CUSTOMERS; id++)
        threads[id + NUM_BARBERS] = thread(Customer, id + NUM_BARBERS);

    /* Merge all threads to the main thread */
    for (int id = 0; id < threadnum; id++)
        threads[id].join();

    // WHY ISN'T THIS PRINTING?
    cout << "Completed barbershop example!\n";
    cout << endl;

    return 0;
}
