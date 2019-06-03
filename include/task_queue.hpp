#ifndef GUL_MINI_UTIL_TASK_QUEUE_INC
#define GUL_MINI_UTIL_TASK_QUEUE_INC

#include <memory>
#include <deque>
#include <functional>
#include <utility>
#include <mutex>
#include <condition_variable>

namespace MiniUtils {

struct TaskBase {
    virtual ~TaskBase() {}
    virtual void run() = 0;
};

template <typename Fn>
struct Task: public TaskBase {
    Task(Fn &&fn): f_(std::forward<Fn>(fn)) {}

    virtual void run() { f_(); }

    Fn f_;
};

typedef std::shared_ptr<TaskBase> TaskPtr;

template <typename Fn>
std::shared_ptr<Task<Fn>> make_task_aux(Fn &&fn)
{
    return std::make_shared<Task<Fn>>(std::forward<Fn>(fn));
}

template <typename ...Args>
std::shared_ptr<TaskBase> make_task(Args &&...args)
{
    return make_task_aux(std::bind(std::forward<Args>(args)...));
}

typedef std::deque<TaskPtr> TaskPtrList;

class TaskQueue: public std::deque<std::shared_ptr<TaskBase>> {
private:
    std::mutex queue_mtx_;
    std::condition_variable queue_cv_;

public:
    template <typename ...Args>
    void push_task(Args &&...args)
    {
        this->push_task(make_task(std::forward<Args>(args)...));
    }

    void push_task(std::shared_ptr<TaskBase> task) 
    {
        std::lock_guard<std::mutex> lck(queue_mtx_);
        this->push_back(task);
        queue_cv_.notify_one();
    }

	std::shared_ptr<TaskBase> pop_task() 
    {
        std::unique_lock<std::mutex> lck(queue_mtx_);
        while (this->empty()) {
            queue_cv_.wait(lck);
        }
        auto task = this->front();
        this->pop_front();
        return task;
	}

    void pop_task(std::deque<std::shared_ptr<TaskBase>> &task_list)
    {
        std::unique_lock<std::mutex> lck(queue_mtx_);
        while (this->empty()) {
            queue_cv_.wait(lck);
        }
        this->swap(task_list);
    }
};

}   // namespace MiniUtils

#endif
