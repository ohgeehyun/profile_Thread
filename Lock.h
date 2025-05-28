#pragma once

/*---------------------------
            LOCK
//c++에서 지원하는 기본 락의 경우 재귀적으로 걸 수 없음.
//또한 데이터를 읽기만 하는데도 lock을 거는건 좋은 현상이 아니다.
사용된 락 정책
// W->R (O)
// R->W (X)
---------------------------*/

class Lock
{
    enum : uint32
    {
        ACQUIRE_TIMEOUT_TICK = 10000,//최대로 기다려줄 시간
        MAX_SPIN_COUNT = 5000,//최대 스핀 횟수
        WRITE_THREAD_MASK = 0xFFFF'0000,//writeflag
        READ_COUNT_MASK = 0x0000'FFFF,//readflag
        EMPTY_FLAG = 0X0000'0000
    };

public:
    void WriteLock(const char* name);
    void WriteUnlock(const char* name);
    void ReadLock(const char* name);
    void ReadUnlock(const char* name);

private:
    Atomic<uint32>_lockFlag;
    uint16 _writeCount = 0;
};

/*----------------------
        LockGuard
-----------------------*/

class ReadLockGuard
{
public:
    ReadLockGuard(Lock& lock, const char* name) :_lock(lock), _name(name) { _lock.ReadLock(_name); }
    ~ReadLockGuard() { _lock.ReadUnlock(_name); }

private:
    Lock& _lock;
    const char* _name;
};

class WriteLockGuard
{
public:
    WriteLockGuard(Lock& lock, const char* name) :_lock(lock), _name(name) { _lock.WriteLock(_name); }
    ~WriteLockGuard() { _lock.WriteUnlock(_name); }


private:
    Lock& _lock;
    const char* _name;
};