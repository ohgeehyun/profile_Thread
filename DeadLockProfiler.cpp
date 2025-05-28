#include "pch.h"
#include "DeadLockProfiler.h"
#include "CoreTLS.h"

void DeadLockProfiler::PushLock(const char* name)
{
    LockGuard guard(_lock);

    int32 lockId = 0;

    //이름이 발견이 된다면 이전 단게에서 발견된 스레드
    auto findIt = _nameToId.find(name);

    //발견된 적이 없어 처음 발견된 스레드
    if (findIt == _nameToId.end())
    {
        lockId = static_cast<int32>(_nameToId.size());
        _nameToId[name] = lockId;
        _idToName[lockId] = name;
    }
    else
    {
        lockId = findIt->second;
    }
    // 잡고 있는 락의 소지 여부
    if (LLockStack.empty() == false)
    {
        //기존에 발견되지 않은 케이스라면 데드락 여부 다시 확인
        const int32 prevId = LLockStack.top();
        if (lockId != prevId)
        {
            set<int32>& history = _lockHistory[prevId];
            if (history.find(lockId) == history.end())
            {
                //처음 발견한 락
                history.insert(lockId);
                CheckCycle();
            }
        }
    }

    LLockStack.push(lockId);
}

void DeadLockProfiler::PopLock(const char* name)
{
    LockGuard guard(_lock);

    if (LLockStack.empty())
        CRASH("MULTIPLE_UNLOCK")

    //최근에 락을 건 락 ID와 언락하는 락의 ID가 일치하지않음.
    int32 lockId = _nameToId[name];

    if (LLockStack.top() != _nameToId[name])
        CRASH("INVALID_UNLOCK")

        LLockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
    const int32 lockCount = static_cast<int32>(_nameToId.size());

    //Init
    _discoveredOrder = vector<int32>(lockCount, -1); // -1 = 방문하지않음
    _discoveredCount = 0;
    _finished = vector<bool>(lockCount, false);
    _parent = vector<int32>(lockCount, -1);
    for (int32 lockId = 0; lockId < lockCount; lockId++)
    {
        Dfs(lockId);
    }

    _discoveredOrder.clear();
    _finished.clear();
    _parent.clear();
}

void DeadLockProfiler::Dfs(int32 here)
{
    if (_discoveredOrder[here] != -1)
        return;

    _discoveredOrder[here] = _discoveredCount++;

    //모든 인접한 정점을 순회
    auto findIt = _lockHistory.find(here);
    if (findIt == _lockHistory.end())
    {
        //락을 소유한 상태에서 다른 락을 잡은 적이 없다.
        _finished[here] = true;
        return;
    }

    set<int32>& nextSet = findIt->second;

    for (int32 there : nextSet)
    {
        //아직 방문한 적이 없다면 방문
        if (_discoveredOrder[there] == -1)
        {
            _parent[there] = here;
            Dfs(there);
            continue;
        }

        //here가 there보다 먼저 발견된다면 there는 here의 후손(순방향)
        if (_discoveredOrder[here] < _discoveredOrder[there])
            continue;

        //순방향이 아니고 ,Dfs(there)가 아직 종료되지 않았다면, there는 here의 선조
        if (_finished[there] == false)
        {
            printf("%s -> %s\n", _idToName[here], _idToName[there]);

            int32 now = here;
            while (true)
            {
                printf("%s -> %s\n", _idToName[_parent[now]], _idToName[now]);
                now = _parent[now];
                if (now == there)
                    break;
            }
        }

        CRASH("DEADLOCK_DETECTED")
    }
    _finished[here] = true;
}