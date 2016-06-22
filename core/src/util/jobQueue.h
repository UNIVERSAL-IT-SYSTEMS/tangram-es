#pragma once

#include <functional>
#include <thread>
#include <unordered_map>
#include <vector>

namespace Tangram {

// JobQueue allows you to send runnable jobs to a target thread:
//   1. Set the target thread for the JobQueue by calling makeCurrentThreadTarget() once on the target thread.
//   2. From any thread, call add() with the job you want to run on the target thread.
//   3. Call JobQueue::runJobsForCurrentThread() on the target thread.
// This is useful for OpenGL resources that must be created and destroyed on the GL thread.

class JobQueue {

using Job = std::function<void()>;
using ThreadId = std::thread::id;

public:
    JobQueue() = default;
    ~JobQueue() = default;

    void makeCurrentThreadTarget();
    void add(Job job);

    static void runJobsForCurrentThread();

private:
    ThreadId m_threadId;

    static std::unordered_map<ThreadId, std::vector<Job>> jobsForThreads;

};

}
