#pragma once
#include <stack>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

/*-----------------------
     DeadLockProfiler
그래프 DFS탐색을 사용하용하여
순방향 교차 역방향중 역방향간선을 찾아서 순환이 일어날 부분을 디버그 단계에서 잡기 위함
------------------------*/

class DeadLockProfiler
{
public:
    void PushLock(const char* name);
    void PopLock(const char* name);
    void CheckCycle();

private:
    void Dfs(int32 index);

private:
    //내부적으로 연산을 할때에는 char보다 int가 빠르기 때문에 2가지 생성
    //cpu 의 레지스터,cpu의 캐시 메모리등이 int에 더적합한 사이즈를 처리하기 때문 char는 너무 크기가 작다.
    unordered_map<const char*, int32>	_nameToId;
    unordered_map<int32, const char*>	_idToName;

    //정점들이 어느 정점으로 간지 기록
    map <int32, set<int32> > _lockHistory;

    Mutex					 _lock;
private:
    vector<int32>	_discoveredOrder;//정점이 발견된 순서를 기록
    int32			_discoveredCount = 0;
    vector<bool>	_finished; //Dfs(i)가 종료 되었는지 여부
    vector<int32>	_parent; //발견된 정점의 부모 정점
};