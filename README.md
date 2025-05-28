# DeadLockProfiler 기반 커스텀 락 

멀티스레드 환경에서 락의 충돌과 데드락을 효과적으로 방지하기 위해 제작된 커스텀 락 시스템입니다.  
읽기/쓰기 락 분리 및 **DFS 기반 데드락 탐지 기능**이 포함되어 있어 디버깅과 안정성 확보에 유리합니다.

---
##flowchart


```mermaid
flowchart TD
    A[스레드 시작] --> B{WriteLock or ReadLock}
    
    B -->|WriteLock| C[자기 자신이 소유 중?]
    C -->|Yes| D[WriteCount 증가 후 리턴]
    C -->|No| E[CAS로 락 플래그 획득 시도]
    E --> F{획득 성공?}
    F -->|Yes| G[WriteCount 증가]
    F -->|No| H[스핀 + 타임아웃 대기]
    H --> E
    G --> I[PushLock(name)]

    B -->|ReadLock| J[자기 자신이 Write 소유?]
    J -->|Yes| K[ReadCount 증가 후 리턴]
    J -->|No| L[CAS로 ReadCount 증가 시도]
    L --> M{성공?}
    M -->|Yes| N[PushLock(name)]
    M -->|No| O[스핀 + 타임아웃 대기]
    O --> L

    I --> P[락 획득 완료]
    N --> P

    P --> Q[업무 처리]
    Q --> R[락 해제 → PopLock(name)]
```
---

## 구성 요소

### Lock

- 읽기/쓰기 락을 분리하여 관리
- CAS 기반 스핀락 구현
- 타임아웃 대기 및 재귀적 쓰기 락 허용

### LockGuard

- `RAII` 방식으로 자동 Lock/Unlock
- `ReadLockGuard`, `WriteLockGuard` 제공

### DeadLockProfiler

- 락 획득 시 락 간 관계 그래프 구성
- DFS를 통해 사이클(= 데드락) 감지
- 디버그 시 `CRASH("DEADLOCK_DETECTED")`로 종료

---

## 락 정책

| 구간             | 허용 여부 | 설명                      |
|------------------|-----------|---------------------------|
| Write → Read     | ✅ 허용    | 동일 스레드 한정          |
| Read → Write     | ❌ 금지    | 데드락 발생 가능성 때문    |
| 중첩 WriteLock    | ✅ 허용    | 동일 스레드인 경우         |
| Read 다중 허용    | ✅ 허용    | 동시에 여러 스레드 가능     |

---
