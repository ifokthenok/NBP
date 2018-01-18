#include <stdio.h>
#include <thread>
#include "utils.h"
#include "state.h"
#include "clock.h"


using namespace std;

void push(Queue<int>& q) {
    bool ok;
    ok = q.push(4);
    printf("push 4, ok=%d\n", ok);
    ok = q.push(5, 1000);
    printf("push 5, ok=%d\n", ok);
    ok = q.push(6, 1000);
    printf("push 6, ok=%d\n", ok);
    ok = q.push(7, 1000);
    printf("push 7, ok=%d\n", ok);
}

void pop(Queue<int>& q) {
    bool ok;
    int e = 0;
    ok = q.pop(e, 1000);
    printf("pop ok=%d, e=%d\n", ok, e);
    ok = q.pop(e, 1000);
    printf("pop ok=%d, e=%d\n", ok, e);
    ok = q.pop(e, 1000);
    printf("pop ok=%d, e=%d\n", ok, e);
}


int main(int argc, char* argv[]) {
    // Queue<int> q(1);
    // thread push_thread(push, ref(q));
    // thread pop_thread(pop, ref(q));
    // push_thread.join();
    // pop_thread.join();

    auto compare = [](int a, int b) { return a < b; };

    PriorityQueue<int, decltype(compare)> pq(3, compare);
    pq.push(1);
    pq.push(100);
    pq.push(10);
    int e = 0;
    bool ok;
    ok = pq.pop(e, 1000);
    printf("pop ok=%d, e=%d\n", ok, e);
    ok = pq.pop(e, 1000);
    printf("pop ok=%d, e=%d\n", ok, e);
    ok = pq.pop(e, 1000);
    printf("pop ok=%d, e=%d\n", ok, e);
    ok = pq.pop(e, 1000);
    printf("pop ok=%d, e=%d\n", ok, e);

    Clock clock;
    int64_t t1 = clock.absoluteTime();
    printf("absolute time=%lldms\n", t1);

    States states;
    printf("current: %s\n", toString(states.getCurrent()).c_str());
    printf("next: %s\n", toString(states.getNext()).c_str());
    printf("pending: %s\n", toString(states.getNext()).c_str());

    int64_t t2 = clock.absoluteTime();
    printf("duration time=%lldms\n", t2 - t1);

    return 0;
}