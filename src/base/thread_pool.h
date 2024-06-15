/**
 * @mainpage 纯cpp11手写线程池，（互斥锁，条件变量，原子变量，手动实现cpp17
 * any及cpp20 信号量）
 * @file threadPool.h
 * @brief 纯cpp11手写线程池，（互斥锁，条件变量，原子变量，手动实现cpp17
 * any及cpp20 信号量）
 * @author xf
 * @version bate 1.1
 * @date 2022-03-09
 * @copyright bsd
 */

#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <iostream>
#include <condition_variable>
#include <memory>
#include <unordered_map>
#include <future>
#include <functional>

const size_t INIT_THREAD_COUNT =
    std::thread::hardware_concurrency() * 2; ///< 默认初始化线程池线程数量
constexpr size_t THRESHOLD = 1024*1024;       ///< 默认任务最大阈值
constexpr size_t THREAD_SIZE_THRESH_HOLD = 100; ///< 默认线程池线程最大阈值
constexpr size_t THREAD_MAX_IDLE_TIME = 60; ///< 默认线程最大空闲时间s

/**
 * @brief 线程池模式\n
 * 两种线程池模式,一种是固定数量,一种动态增减
 */
enum class CPoolMode
{
    MODE_FIXED, ///< 固定数量
    MODE_CACHED ///< 动态数量
};

/**
 * @brief 线程简单包装类\n
 * 封装了线程的启动函数,以及线程id
 */
class CThread
{
    using CALLBACK_FUNC = std::function<void(int)>;

public:
    CThread(CALLBACK_FUNC parameter)
        : callbackFunc_(parameter),
          threadId_(numbers++)
    {
        std::cout << "create thread\n";
    }
    ~CThread() { std::cout << "release thread\n"; }

    /// @brief 启动线程
    void Start();

    /// @brief 获取线程编号
    /// @return 线程编程
    int Getid() const;

private:
    CALLBACK_FUNC callbackFunc_; ///> 待执行函数对象
    int threadId_;               ///> 线程id
    static int numbers;          ///> 线程编号
};
/**
 * @brief 纯cpp11手写线程池，（互斥锁，条件变量，原子变量，手动实现cpp17
 * any及cpp20 信号量）
 */
class CThreadPool
{
private:
    size_t count_;                //初始线程数量
    size_t taskMax_;              //任务上限阙值
    size_t threadSizeThreshHold_; //线程上限阙值
    std::atomic_uint taskSize_;   //任务数量
    CPoolMode mode_;              //线程池模式
    // std::vector<std::unique_ptr<CThread>>	m_arr;			//线程队列
    std::unordered_map<int, std::unique_ptr<CThread>> threadQue_; //线程队列
    using CTask = std::function<void()>;
    std::queue<CTask> taskQue_;        //任务队列
    std::atomic_bool isConfirm_;       //是否依旧确定
    std::mutex queMut_;                //任务队列操作锁
    std::condition_variable notFull_;  //任务队列不满
    std::condition_variable notEmpty_; //任务队列不空
    std::atomic_uint idleThreadSize_;  //空闲线程数量
    std::atomic_int currThreadSize_;   //当前线程数量
public:
    /// @brief 设置线程池模式
    /// @param parameter CPoolMode枚举类型
    void SetMode(CPoolMode parameter);

    // CResult AddTask(std::shared_ptr<CTask>);				//添加任务
    /**
     * @brief 线程池添加任务模板函数
     * @param func 函数对象
     * @param args 函数参数(不定量)
     * @return 返回一个future,其内包装了函数的返回值
     */
    template <typename Func, typename... Args>
    auto AddTask(Func&& func, Args&&... args)
        -> std::future<decltype(func(args...))>
    {
        using RType = decltype(func(args...));
        auto task = std::make_shared<std::packaged_task<RType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
        std::future<RType> result = task->get_future();

        std::unique_lock<std::mutex> lock(queMut_);
        //线程的通信  等待任务队列有空余   wait   wait_for   wait_until
        // 用户提交任务，最长不能阻塞超过1s，否则判断提交任务失败，返回
        bool res = notFull_.wait_for(
            lock, std::chrono::seconds(1),
            [this]() -> bool { return taskQue_.size() < taskMax_; });
        if (!res)
        {
            auto _task = std::make_shared<std::packaged_task<RType()>>(
                []() -> RType { return RType(); });
            (*_task)();
            return _task->get_future();
        }

        // 如果有空余，把任务放入任务队列中
        // m_que.emplace(sp);//添加任务
        taskQue_.emplace([task]() { (*task)(); });
        taskSize_++;
        // 因为新放了任务，任务队列肯定不空了，在notEmpty_上进行通知，赶快分配线程执行任务
        notEmpty_.notify_one();

        // cached模式 任务处理比较紧急 场景：小而快的任务
        // 需要根据任务数量和空闲线程的数量，判断是否需要创建新的线程出来
        if (mode_ == CPoolMode::MODE_CACHED //动态方式
            && idleThreadSize_ < taskSize_  //空闲线程小于任务线程
            &&
            currThreadSize_ < threadSizeThreshHold_) //当前线程数量小于线程阈值
        {
            //创建新的线程对象
            std::unique_ptr<CThread> thread = std::make_unique<CThread>(
                std::bind(&CThreadPool::CallThreadFunction, this,
                          std::placeholders::_1));
            thread->Start(); // 启动线程
            threadQue_.emplace(thread->Getid(), std::move(thread));
            // 修改线程个数相关的变量
            idleThreadSize_++; //空闲数量加一
            currThreadSize_++; //线程总数量加一
        }
        return result; // 返回任务的Result对象
    }

    /// @brief 启动线程池
    /// @param count 初始线程数量,默认为cpu核心数量
    void Start(int count = INIT_THREAD_COUNT);

    /// @brief 设置任务上限阙值
    /// @param threshhold 阙值数值
    void SetTaskQueMaxThreshold(int threshhold);

    /// @brief 设置线程上限阙值
    /// @param threshhold  阙值数值
    void SetThreadSizeThreshHold(size_t threshhold);

    /// @brief 线程执行函数
    /// @param 线程编号
    void CallThreadFunction(int);
    CThreadPool();
    ~CThreadPool();
};

/*
class CResult;
//任务类
class CTask
{
private:
    //任务结果指针
    CResult* m_result = nullptr;
public:
    //设置任务结果
    void setResult(CResult* p);
    virtual CAny CallFunction() { return nullptr; }
    //执行任务
    void exec();
};

//cpp 17 Any类型
class CAny
{
private:
    //基类
    class BASE {
    public:
        virtual ~BASE() = default;
    };

    //派生类
    template <typename T>
    class CDerive : public BASE
    {
    public:
        CDerive(T _data) :data(_data) {}
        T data;
    };
public:
    template <typename T>
    CAny(T val) :m_data(std::make_unique<CDerive<T>>(val)) {}
    CAny() = default;
    ~CAny() = default;
    CAny& operator=(const CAny&) = delete;
    CAny& operator=(CAny&&) = default;
    CAny(const CAny&) = delete;
    CAny(CAny&&) = default;

    //获取返回结果
    template <typename T>
    T cast()
    {
        CDerive<T>* p = dynamic_cast<CDerive<T>*>(m_data.get());
        if (p == nullptr)
            throw "类型不匹配，转换失败";
        return p->data;
    }
private:
    //存储结果的基类指针
    std::unique_ptr<BASE> m_data;
};

//信号量
class emaphore
{
private:
    std::mutex					m_mutex;
    std::condition_variable		m_CV;
    size_t						count;						//信号量
public:
    emaphore(size_t _count = 0) :count(_count) {}
    //获取一个信号
    void wait() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_CV.wait(lock, [this]()->bool {return count > 0; });
        count--;
    }
    //投入一个信号
    void post()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        count++;
        m_CV.notify_all();
    }
};

//结果类
class CResult
{
private:
    CAny						m_any;			//结果值
    std::atomic_bool			m_bl;			//是否有效
    std::shared_ptr<CTask>		m_sp;			//任务指针
public:
    CResult(std::shared_ptr<CTask> sp, bool isValue = false) :m_sp(sp),
m_bl(isValue) { m_sp->setResult(this); } CResult(const CResult& val); ~CResult()
= default; void SetTask(CAny any);						//设置结果 CAny Get();
//获取结果 emaphore					m_emaphore;		//线程通知
};
*/