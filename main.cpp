#include <windows.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstdio>
using namespace std;


atomic<bool> running(true);
void keybindListener() {
    while (running) {
        bool ctrl  = GetAsyncKeyState(VK_CONTROL) & 0x8000;
        bool shift = GetAsyncKeyState(VK_SHIFT)   & 0x8000;
        bool l     = GetAsyncKeyState('L')        & 0x8000;
        
        if (ctrl && shift && l) {
            printf("[+] ------------------------ Keybind pressed, program terminating ------------------------\n");
            running = false;
            exit(0);
        }

        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

constexpr int delay = 10;
void postureReminder() {
    int localDelay = delay;
    unsigned long long runtime = 0;
    unsigned long long iteration = 0;
    while (running) {
        if (localDelay > 0) {
            printf("[+] Delayed 1 second | Sseconds left: %d | Total seconds elapsed: %llu\n", localDelay, runtime);

            this_thread::sleep_for(chrono::seconds(1));
            localDelay--;
        } else {
            printf("[+] Iteration %llu\n", ++iteration);

            localDelay = delay;
        }
        runtime++;
    }
}


int main() {
    printf("[+] --------------------- Entrypoint triggered --------------------- \n");

    thread listener(keybindListener);
    thread reminder(postureReminder);

    reminder.join();
    listener.detach();

    throw("[-] --------------------- !!! Main thread terminated unexpectedly !!! --------------------- \n");
    return 0;
}
