#pragma once
#include<functional>

//전역으로 스레드를 관리해줄 스레드 매니저

class ThreadManager
{
public:
    ThreadManager();
    ~ThreadManager();

    void Launch(function<void(void)> callback);
    void Join();

    static void InitTLS();
    static void DestroyTLS();

    static void DoGlobalQueueWork();
    static void DistributeReservedJobs();
private:
    Mutex _lock;
    vector<thread> _threads;
};