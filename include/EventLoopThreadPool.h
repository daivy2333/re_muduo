#pragma once
#include "noncopyable.h"
#include<functional>
#include <string>
#include<vector>
#include<memory>
#include "Eventloop.h"

class EventLoopThread;
using namespace std;

class EventLoopThreadPool : noncopyable
{
public:
    using ThreadInitCallback = function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop *baseLoop, const string &nameArg);
    ~EventLoopThreadPool();

    void setThreadNUm(int numThreads) {numThreads_ = numThreads;}

    void start(const ThreadInitCallback &cb = ThreadInitCallback());

    EventLoop* getNextLoop();

    vector<EventLoop*> getAllLoops();

    bool started() const {return started_;}
    const string name() const {return name_;}
private:
    EventLoop *baseLoop_;
    string name_;
    bool started_;
    int numThreads_;
    int next_;
    vector<unique_ptr<EventLoopThread>> threads_;
    vector<EventLoop*> loops_;
};