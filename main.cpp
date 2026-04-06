#include <windows.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstdio>
using namespace std;


/* Adjustable variable for delay between posture reminders */
constexpr int delay = 10;

/* Shared variable for terminating entire program */
atomic<bool> running(true);
/* Listener for keybind */
void keybindListener() {
    while (running) {
        bool ctrl  = GetAsyncKeyState(VK_CONTROL) & 0x8000;
        bool shift = GetAsyncKeyState(VK_SHIFT)   & 0x8000;
        bool l     = GetAsyncKeyState('L')        & 0x8000;
        
        // terminate upon keybind pressed
        if (ctrl && shift && l) {
            printf("hopefully terminated :prayge:\n");
            running = false;
            exit(0);
        }

        // sleep for 10ms between checks for stability (100 checks/s)
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

/* main program */
void postureReminder() {
    int localDelay = delay;
    unsigned long long runtime = 0;
    unsigned long long iteration = 0;
    while (running) {
        if (localDelay > 0) {
            printf("delayed 1 sec | seconds left: %d | thread's runtime: %llu\n", localDelay, runtime);

            // delay 1 second
            this_thread::sleep_for(chrono::seconds(1));
            localDelay--;
        } else {
            printf("iteration %llu orz bruce\n", iteration);
            
            iteration++;
            // reset delay
            localDelay = delay;
        } 
        runtime++;
    }
}

/* program entrypoint */
int main() {
    printf("program started i think :blobpensivepray:\n");

    thread listener(keybindListener);
    thread reminder(postureReminder);

    reminder.join();
    listener.detach();

    throw("main thread terminated how :blobfearful:\n");
    return 0;
}
