# frt - Flössie's ready (FreeRTOS) threading

frt is an object-oriented wrapper around FreeRTOS tasks, mutexes, semaphores, and queues. It provides the basic tools for a clean multithreading approach based on the [Arduino_FreeRTOS_Library](https://github.com/feilipu/Arduino_FreeRTOS_Library) with focus on static allocation, so you know your SRAM demands at compile time.

This will compile just fine with the stock [Arduino_FreeRTOS_Library](https://github.com/feilipu/Arduino_FreeRTOS_Library), but if you want the advantages of static allocation you are welcome to try my [`minimal-static`](https://github.com/Floessie/Arduino_FreeRTOS_Library/tree/minimal-static) branch with frt.

## Implementation

Just take a look at [`frt.h`](https://github.com/Floessie/frt/blob/master/src/frt.h). It contains the whole wrapper in about 500 lines of C++ code.

## Examples

* [`Blink_AnalogRead.ino`](https://github.com/Floessie/frt/blob/master/examples/Blink_AnalogRead/Blink_AnalogRead.ino): Like the classic [Arduino_FreeRTOS_Library example](https://github.com/feilipu/Arduino_FreeRTOS_Library/blob/master/examples/Blink_AnalogRead/Blink_AnalogRead.ino) this one blinks the builtin LED in one task and prints the value of `A0` in another task. The `loop()` is a bit more sophisticated, as it stops one task after five seconds and prints some statistics.
* [`Queue.ino`](https://github.com/Floessie/frt/blob/master/examples/Queue/Queue.ino): Shows two tasks communicating via a queue at full speed. There's a monitoring task and also a mutex involved. This example invites you to play with priorities and optimize the data flow for lower latencies.
* [`QueueISR.ino`](https://github.com/Floessie/frt/blob/master/examples/QueueISR/QueueISR.ino): Asynchronous ADC via ISR and data transfer to task with a queue. And there's also a monitoring task for fun.
* [`CriticalSection.ino`](https://github.com/Floessie/frt/blob/master/examples/CriticalSection/CriticalSection.ino): Asynchronous ADC via ISR and data transfer to task using *direct to task notification* and a critical section.

## API

The whole API resides in the `frt` namespace. That doesn't mean your classes have to be in namespace `frt`, but that all classes of `frt` have to be prefixed with `frt::` when  using them. See the code snippets below and the examples above.

### Task

A task (or thread in other OSes) is the thing that does the work. It's like your common `loop()` workhorse, but with a real OS you can have many of those `loop()` functions running concurrently. A single task is basically implemented like so:

```c++
class MyFirstTask :
    public frt::Task<MyFirstTask>
{
public:
    bool run()
    {
        // Do something meaningful here...

        return true;
    }
};
```

The repetition of `MyFirstTask` in `frt::Task<MyFirstTask>` is due to [static dispatch per CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern#Static_polymorphism), a commonly used pattern that saves code and precious RAM. Think of `run()` as your previously used `loop()` function with an additional return value, with which you can signal if you want to be called again or not. If you returned `false` in the example above, `run()` would only be called once.

Things you can only do inside your task class:
* `yield()`: If there's another task with the same or higher priority, switch over to it voluntarily.
* `msleep(milliseconds)`: Sleep for some milliseconds. This suspends the task so that others can run.
  - Be aware that there's a [timer granularity](https://github.com/feilipu/Arduino_FreeRTOS_Library#general) called *tick* of usually 15 milliseconds. This function will sleep at least one tick, so that 10 milliseconds will become 15, and round down, so that 40 milliseconds become 30. For a workaround, see the next function.
* `msleep(milliseconds, remainder)`: Sleep for some milliseconds and store the remaining milliseconds in `remainder`. This will result in some jitter but won't loose milliseconds to the timer granularity.
* `wait()`: Wait for a [*direct to task notification*](https://www.freertos.org/RTOS_Task_Notification_As_Binary_Semaphore.html). Some other task must call `post()` to wake you up again.
  - This behaves just like a binary semaphore: A `wait()` will reset all `post()`s that were done before, so the next `wait()` will actually wait until posted again.
* `wait(milliseconds)`: Same as above, but with a timeout. Returns `true` if someone `post()`ed you, or `false` on timeout.
* `wait(milliseconds, remainder)`: Same as above but with the `remainder` mechanism on timeout.
* `beginCriticalSection()`: Start a critical section. Disables interrupts, so you can access and modify (volatile) variables that are also touched in ISRs.
* `endCriticalSection()`: Ends a critical section. Reenables interrupts.

Functions that can be called from outside:
* `start(priority)`: Start the task with a certain priority (higher number = higher priority).
  - Note that there's a limited number of available priorities. Stock Arduino_FreeRTOS_Library supports 0-3, my [`minimal-static`](https://github.com/Floessie/Arduino_FreeRTOS_Library/tree/minimal-static) branch 0-7.
  - The idle task that executes `loop()` has priority 0.
* `stop()`: Stops the task.
  - This blocks until the task left its `run()` function, so if you're blocking there indefinitely, `stop()` will never return.
  - If you want to stop from within your task, return `false` from `run()`. Don't call `stop()`!
  - If you want to stop from the idle task, use `stopFromIdleTask()`.
  - Stopping a task is harder than you might think. Be sure to block with timeouts (on `wait()`, semaphores, or queues) in `run()`.
* `stopFromIdleTask()`: Stops the task from the idle task (your `loop()` implementation).
* `isRunning()`: Returns true if the task is started.
* `getUsedStackSize()`: Each task has a buffer that is used for storing function local variables and return addresses. This function lets you determine the maximum number of bytes used (so far).
  - Only valid while the task is running.
  - Interrupts are also served in a task's context, so the result may vary. Don't be too conservative.
* `post()`: Wake task via *direct to task notification*.
* `preparePostFromInterrupt()`: When posting from an interrupt, this function must be called when entering the ISR.
* `postFromInterrupt()`: Like `post()` but from inside an ISR.
* `finalizePostFromInterrupt()`: This function must be called last in the ISR where you have a `postFromInterrupt()`.
  - It doesn't matter if `postFromInterrupt()` was really called during the ISR. This is remembered internally and handled automatically.

### Mutex

Mutexes protect code sections from being accessed concurrently by multiple tasks. One task *locks* the mutex, so that another task has to wait on the mutex for the first task to *unlock* it. That's not busy waiting in a loop like `delay()` does: The scheduler kicks in and resumes another task, most probably the one who is locking the mutex, because FreeRTOS supports [priority inheritance](https://www.freertos.org/Real-time-embedded-RTOS-mutexes.html). When the first task unlocks the mutex, one of the other tasks waiting on it can proceed.

Normally you would only protect variable accesses and keep the locked times short. But they can as well be used to guard an action that should not be interrupted or a resource that has to finish something before something new is started. Keep an eye on the locking sequence when multiple mutexes are involved: It's easy to shoot oneself in the foot and provoke a [deadlock](https://en.wikipedia.org/wiki/Deadlock). To avoid that either go for broader locking with fewer mutexes, or avoid nested locking by restructuring the code.

Mutexes in FreeRTOS can't be used from ISRs. See `frt::Task::beginCriticalSection()` for that.

A `frt::Mutex` has a dead simple interface:
* `lock()`: Locks the mutex.
* `unlock()`: Unlocks the mutex.

### Semaphore

Semaphores synchronize actions like, "Proceed only when I told you so!" Thus, semaphores are "locked" in pristine state, whereas mutexes are unlocked. Mutexes must be "given back" via `unlock()`, whereas semaphores are "consumed". Usually there's one task `wait()`ing on a semaphore and another one `post()`ing on it.

There are two kinds of semaphores:
1. Binary semaphores, which only remember if they were posted but not how often. This is often sufficient and the default for a `frt::Semaphore`.
2. Counting semaphores, that remember how often they were posted so that the waiting task can proceed exactly that many times without blocking. Such a semaphore is created by passing `true` to the constructor:

```c++
frt::Semaphore my_binary_semaphore;
frt::Semaphore my_counting_semaphore(true);
```

If you want to share data via a buffer (and don't want to use `frt::Queue`), you need a mutex to protect the buffer and a semaphore to notify the consumer. When sharing between an ISR and a task, you can't use a mutex, but must employ a *critical section*.

These are the functions of a semaphore:
* `wait()`: Wait indefinitely for someone posting the semaphore.
* `wait(milliseconds)`: Wait with timeout (at least one tick).
* `wait(milliseconds, remainder)`: Same as above but with the `remainder` mechanism on timeout.
* `post()`: Wake the task waiting on the semaphore.
* `preparePostFromInterrupt()`: When posting from an interrupt, this function must be called when entering the ISR.
* `postFromInterrupt()`: Like `post()` but from inside an ISR.
* `finalizePostFromInterrupt()`: This function must be called last in the ISR whether you called `postFromInterrupt()` or not.

### Queue

Queues let you pass data from one task to another or to and from ISRs. They can hold up to a fixed number of items, and those items have a fixed type, too. But don't worry: This scheme is flexible enough for almost everything. See the following example of a queue holding up to five compound items:

```c++
struct Item {
    char key[6];
    int value;
    unsigned long timestamp;
};

frt::Queue<Item, 5> my_queue;
```

Here's the interface of `frt::Queue`:
* `getFillLevel()`: Returns the number of items, the queue is currently holding.
* `push(item)`: Adds one item to the back of the queue. This will wake the task waiting for data. If there's no space left, this variant of `push()` will wait forever until another task pops an item from the queue.
* `push(item, milliseconds)`: Same as above, but with a timeout (at least one tick).
* `push(item, milliseconds, remainder)`: Same as above, but with the remainder mechanism.
* `preparePushFromInterrupt()`: When pushing from an interrupt, this function must be called when entering the ISR.
* `pushFromInterrupt(item)`: Like `push()` but from inside an ISR. Doesn't wait but returns `false` if no space left.
* `finalizePushFromInterrupt()`: This function must be called last in the ISR no matter if you called `postFromInterrupt()` or not.
* `pop(item)`: Pop item from the front of the queue. This will wake a task that stalled on the filled queue. Will wait forever until another task pushes an item.
* `pop(item, milliseconds)`: Same as above, but with a timeout (at least one tick).
* `pop(item, milliseconds, remainder)`: Same as above, but with the remainder mechanism.
* `preparePopFromInterrupt()`: When popping from an interrupt, this function must be called when entering the ISR.
* `popFromInterrupt(item)`: Like `pop()` but from inside an ISR. Doesn't wait but returns `false` if there is nothing to pop.
* `finalizePopFromInterrupt()`: This function must be called last in the ISR no matter if you called `popFromInterrupt()` or not.

## Remarks about the API

Maybe you miss some functions from the API. If so, there might be several reasons why they are missing:
* Values that are specified by you or your Arduino_FreeRTOS_Library configuration aren't exposed because you already know them.
* A wrapper for [`vTaskDelayUntil()`](https://www.freertos.org/vtaskdelayuntil.html) isn't available, as this would mean exposing the *tick* while `frt` tries its best to hide it. You can use `millis()` to determine if you missed a deadline or if there's enough time left for a `frt::Task::msleep()` as long as timer 0 wasn't stopped in the meantime.
* The FreeRTOS function is too special or the problem can be solved by other means.
* I simply didn't deem the function to be important enough.

