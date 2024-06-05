#include "thread_pool.h"
#include <iostream>
int CThread::numbers = 0;

// 设置线程池的工作模式
void CThreadPool::SetMode(CPoolMode parameter)
{
    if (!isConfirm_) mode_ = parameter;
}

// 给线程池提交任务    用户调用该接口，传入任务对象，生产任务
// CResult CThreadPool::AddTask(std::shared_ptr<CTask> sp)
//{
//	std::unique_lock<std::mutex> lock(queMut);
//	//线程的通信  等待任务队列有空余   wait   wait_for   wait_until
//	// 用户提交任务，最长不能阻塞超过1s，否则判断提交任务失败，返回
//	bool res = notFull.wait_for(lock, std::chrono::seconds(1),
//		[this]()->bool { return que.size() < taskMax; });
//	if (!res)
//		// return task->getResult();  // Task  Result
//线程执行完task，task对象就被析构掉了 		return CResult(sp);
//	// 如果有空余，把任务放入任务队列中
//	que.emplace(sp);//添加任务
//	taskSize++;
//	//
//因为新放了任务，任务队列肯定不空了，在notEmpty_上进行通知，赶快分配线程执行任务
//	notEmpty.notify_all();
//
//	// cached模式 任务处理比较紧急 场景：小而快的任务
//需要根据任务数量和空闲线程的数量，判断是否需要创建新的线程出来 	if (mode ==
//CPollMode::MODE_CACHED				//动态方式
//		&& idleThreadSize < taskSize				//空闲线程小于任务线程
//		&& currThreadSize < threadSizeThreshHold)	//当前线程数量小于线程阈值
//	{
//		//创建新的线程对象
//		std::unique_ptr<CThread> thread =
//std::make_unique<CThread>(std::bind(&CThreadPool::CallThreadFunction, this,
//std::placeholders::_1)); 		thread->start();// 启动线程
//		arr.emplace(thread->Getid(), std::move(thread));
//		// 修改线程个数相关的变量
//		idleThreadSize++;		//空闲数量加一
//		currThreadSize++;		//线程总数量加一
//	}
//	return CResult(sp, true);// 返回任务的Result对象
//}

// 开启线程池
void CThreadPool::Start(int count)
{
    isConfirm_ = true; // 设置线程池的运行状态
    // 记录初始线程个数
    count = count;
    currThreadSize_ = count;
    // 创建线程对象
    for (size_t i = 0; i < count; i++)
    {
        // 创建thread线程对象的时候，把线程函数给到thread线程对象
        std::unique_ptr<CThread> thread = std::make_unique<CThread>(std::bind(
            &CThreadPool::CallThreadFunction, this, std::placeholders::_1));
        threadQue_.emplace(thread->Getid(), std::move(thread));
    }
    // 启动所有线程
    for (int i = 0; i < count; i++)
    {
        threadQue_[i]->Start();
        idleThreadSize_++;
    }
}

// 设置task任务队列上线阈值
void CThreadPool::SetTaskQueMaxThreshold(int threshhold)
{
    if (!isConfirm_) taskMax_ = threshhold;
}

// 设置线程池cached模式下线程阈值
void CThreadPool::SetThreadSizeThreshHold(size_t threshhold)
{
    if (!isConfirm_ && mode_ == CPoolMode::MODE_CACHED)
        threadSizeThreshHold_ = threshhold;
}

// 定义线程函数   线程池的所有线程从任务队列里面消费任务
void CThreadPool::CallThreadFunction(int threadId)
{
    auto lastTime = std::chrono::steady_clock::now();
    while (taskSize_ !=
           -1) // 所有任务必须执行完成，线程池才可以回收所有线程资源
    {
        // std::shared_ptr<CTask> sp;
        CTask sp;
        {
            std::unique_lock<std::mutex> lock(queMut_);
            if (mode_ == CPoolMode::MODE_CACHED)
            {
                while (taskQue_.size() == 0)
                {
                    if (std::cv_status::timeout ==
                        notEmpty_.wait_for(lock, std::chrono::seconds(1)))
                    {
                        auto time =
                            std::chrono::duration_cast<std::chrono::seconds>(
                                std::chrono::steady_clock::now() - lastTime);
                        if (time.count() > THREAD_MAX_IDLE_TIME &&
                            currThreadSize_ > count_)
                        {
                            threadQue_.erase(threadId);
                            idleThreadSize_--; //空闲数量减一
                            currThreadSize_--; //线程总数量减一
                            return;
                        }
                    }
                }
            }
            else
            {
                // 等待notEmpty条件
                notEmpty_.wait(lock,
                               [this]() -> bool { return !taskQue_.empty(); });
            }
            idleThreadSize_--;
            // 从任务队列种取一个任务出来
            sp = taskQue_.front();
            taskQue_.pop();
            taskSize_--;
            // 如果依然有剩余任务，继续通知其它得线程执行任务
            if (!taskQue_.empty()) notEmpty_.notify_one();
            // 取出一个任务，进行通知，通知可以继续提交生产任务
            notFull_.notify_one();
        }                        //释放锁
        if (sp != nullptr) sp(); // 执行任务；把任务的返回值setVal方法给到Result
        idleThreadSize_++;
        lastTime = std::chrono::steady_clock::now(); // 更新线程执行完任务的时间
    }
}
// 线程池构造
CThreadPool::CThreadPool()
    : count_(INIT_THREAD_COUNT),
      taskMax_(THRESHOLD),
      taskSize_(0),
      mode_(CPoolMode::MODE_FIXED),
      isConfirm_(false),
      threadSizeThreshHold_(THREAD_SIZE_THRESH_HOLD),
      idleThreadSize_(0),
      currThreadSize_(0)
{
}

CThreadPool::~CThreadPool() { taskSize_ = -1; }

////////////////////////////////////////////////////////////////////////
void CThread::Start()
{
    std::thread th(callbackFunc_, Getid());
    th.detach();
}

int CThread::Getid() const { return threadId_; }

// void CTask::setResult(CResult* p)
//{
//	result = p;
// }

// void CTask::exec()
//{
//	if (result != nullptr) {
//		result->SetTask(CallFunction());
//	}
// }
//
// CResult::CResult(const CResult& temp)
//{
//	sp = temp.sp;
//	any = std::move(const_cast<CResult*>(&temp)->any);
//	bl = temp.bl.load();
// }
//
// void CResult::SetTask(CAny any)
//{
//	any = std::move(any);
//	emaphore.post();
// }
//
// CAny CResult::Get()
//{
//	if (!bl) return "";
//	emaphore.wait();
//	return std::move(any);
// }