#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>

struct Mutex {
    Mutex() {
        pthread_mutex_init(&mutex, 0);
    }
    ~Mutex() {
        pthread_mutex_destroy(&mutex);
    }
    void lock() {
        pthread_mutex_lock(&mutex);
    }
    void unlock() {
        pthread_mutex_unlock(&mutex);
    }
    void* operator()() {
        return &mutex;
    }
private:
    pthread_mutex_t mutex;
};

struct ScopedLock {
    ScopedLock(Mutex& mutex) : lock(mutex) {
        lock.lock();
    }
    ~ScopedLock() {
        lock.unlock();
    }
private:
    Mutex& lock;
};

struct ConditionVariable {
    ConditionVariable() {
        pthread_cond_init(&condition, 0);
    }
    ~ConditionVariable() {
        pthread_cond_destroy(&condition);
    }
    void wait(Mutex& mutex) {
        pthread_cond_wait(&condition, (pthread_mutex_t*)mutex());
    }
    void signal() {
        pthread_cond_signal(&condition);
    }
private:
    pthread_cond_t condition;
};

template <typename T>
class Queue
{
public:
    bool empty() const {
        std::unique_lock<std::mutex> lock(m);
        return q.empty();
    }

    int size() {
        std::unique_lock<std::mutex> lock(m);
        return q.size();
    }

    void push(const T& e) {
        std::unique_lock<std::mutex> lock(m);
        q.push(e);
        cv.notify_all();
    }

    void pop(T& e) {
        std::unique_lock<std::mutex> lock(m);
        if (q.empty()) {
            cv.wait(lock);
        }
        e = q.front();
        q.pop();
    }

private:
    std::queue<T> q;
    std::mutex m;
    std::condition_variable cv;
};


typedef void* Message;
typedef void (*Handler)(Message message);

struct HandlerThread {
    HandlerThread(Handler h) : handler(h) {}
    HandlerThread() : handler(0) {}

    void setHandler(Handler handler) {
        this->handler = handler;
    }

    void sendMessage(Message message) {
        messageQueue.push(message);
    }

    void start();
    void stop();

    Handler handler;
    Queue<Message> messageQueue;

private:
    std::thread t;
};

static void handleMessages(void* arg) {
    HandlerThread* t = static_cast<HandlerThread*>(arg);
    for (; ;) {
        Message message;
        t->messageQueue.pop(message);
        if (message == NULL) {
            break;
        }
        if (t->handler) {
            t->handler(message);
        }
    }
}

void HandlerThread::start() {
    t = std::thread(handleMessages, this);
}

void HandlerThread::stop() {
    sendMessage(0);
    t.join();
}


void myHandler(Message message) {
    const char* s = (const char*)message;
    if (s)
        printf("myHandler: message=%s\n", s);
    sleep(1);    
}

int main(int argc, char* argv[]) {

    HandlerThread ht(myHandler);
    ht.start();
    char* msg1 = "hello";
    char* msg2 = "world";
    ht.sendMessage(msg1);
    ht.sendMessage(msg2);
    ht.stop();
    return 0;
}

