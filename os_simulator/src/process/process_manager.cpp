#include "../include/process_manager.h"
#include <algorithm>
#include <cstdlib>
#include <queue>
#include <sstream>
#include <iomanip>
#include <windows.h>

// ====================================================================
//  1. 调度辅助计算函数
// ====================================================================
static void calculateMetrics(std::vector<PCB>& finished) {
    for (auto& p : finished) {
        p.turnAroundTime = p.finishTime - p.arrivalTime;
        p.weightedTurnAroundTime = (double)p.turnAroundTime / p.burstTime;
    }
}

static SchedulingResult buildResult(const std::string& algoName, const std::vector<PCB>& finished, const std::vector<std::string>& timeline) {
    SchedulingResult res;
    res.algoName = algoName;
    res.finishedProcesses = finished;
    res.executionTimeline = timeline;
    
    double totalTurnaround = 0;
    double totalWeightedTurnaround = 0;
    for (const auto& p : finished) {
        totalTurnaround += p.turnAroundTime;
        totalWeightedTurnaround += p.weightedTurnAroundTime;
    }
    
    if (!finished.empty()) {
        res.avgTurnAroundTime = totalTurnaround / finished.size();
        res.avgWeightedTurnAroundTime = totalWeightedTurnaround / finished.size();
    }
    return res;
}

// ====================================================================
//  2. 算法实现：先来先服务 (FCFS)
// ====================================================================
SchedulingResult ProcessManager::runFCFS(std::vector<PCB> processes) {
    // 按到达时间升序排序
    std::sort(processes.begin(), processes.end(), [](const PCB& a, const PCB& b) {
        return a.arrivalTime < b.arrivalTime;
    });

    std::vector<PCB> finished;
    std::vector<std::string> timeline;
    int currentTime = 0;

    for (auto& p : processes) {
        if (currentTime < p.arrivalTime) {
            currentTime = p.arrivalTime;
        }
        
        p.status = 'R';
        for (int t = 0; t < p.burstTime; ++t) {
            timeline.push_back(p.name);
        }
        currentTime += p.burstTime;
        p.finishTime = currentTime;
        p.status = 'F';
        
        finished.push_back(p);
    }

    calculateMetrics(finished);
    return buildResult("先来先服务算法 (FCFS)", finished, timeline);
}

// ====================================================================
//  3. 算法实现：短作业优先 (SJF) - 非抢占式
// ====================================================================
SchedulingResult ProcessManager::runSJF(std::vector<PCB> processes) {
    std::vector<PCB> finished;
    std::vector<std::string> timeline;
    int currentTime = 0;
    size_t completedCount = 0;
    size_t total = processes.size();

    std::vector<bool> isFinished(total, false);

    while (completedCount < total) {
        // 寻找当前已到达且 burstTime 最小的就绪作业
        int selectedIdx = -1;
        int minBurst = 1e9;
        int earliestArrival = 1e9;

        for (size_t i = 0; i < total; ++i) {
            if (!isFinished[i] && processes[i].arrivalTime <= currentTime) {
                if (processes[i].burstTime < minBurst) {
                    minBurst = processes[i].burstTime;
                    selectedIdx = i;
                } else if (processes[i].burstTime == minBurst) {
                    // 如果服务时间相同，先来先服务
                    if (processes[i].arrivalTime < processes[selectedIdx].arrivalTime) {
                        selectedIdx = i;
                    }
                }
            }
        }

        // 如果当前没有作业到达，时间推进到下一个最近作业的到达时间
        if (selectedIdx == -1) {
            int nextArrival = 1e9;
            for (size_t i = 0; i < total; ++i) {
                if (!isFinished[i] && processes[i].arrivalTime < nextArrival) {
                    nextArrival = processes[i].arrivalTime;
                }
            }
            currentTime = nextArrival;
            continue;
        }

        auto& p = processes[selectedIdx];
        p.status = 'R';
        for (int t = 0; t < p.burstTime; ++t) {
            timeline.push_back(p.name);
        }
        currentTime += p.burstTime;
        p.finishTime = currentTime;
        p.status = 'F';
        
        isFinished[selectedIdx] = true;
        finished.push_back(p);
        completedCount++;
    }

    calculateMetrics(finished);
    return buildResult("短作业优先算法 (SJF)", finished, timeline);
}

// ====================================================================
//  4. 算法实现：高响应比优先 (HRRN)
// ====================================================================
SchedulingResult ProcessManager::runHRRN(std::vector<PCB> processes) {
    std::vector<PCB> finished;
    std::vector<std::string> timeline;
    int currentTime = 0;
    size_t completedCount = 0;
    size_t total = processes.size();

    std::vector<bool> isFinished(total, false);

    while (completedCount < total) {
        int selectedIdx = -1;
        double maxRatio = -1.0;

        for (size_t i = 0; i < total; ++i) {
            if (!isFinished[i] && processes[i].arrivalTime <= currentTime) {
                // 响应比 = (等待时间 + 服务时间) / 服务时间
                int waitingTime = currentTime - processes[i].arrivalTime;
                double ratio = (double)(waitingTime + processes[i].burstTime) / processes[i].burstTime;
                
                if (ratio > maxRatio) {
                    maxRatio = ratio;
                    selectedIdx = i;
                } else if (ratio == maxRatio) {
                    if (processes[i].arrivalTime < processes[selectedIdx].arrivalTime) {
                        selectedIdx = i;
                    }
                }
            }
        }

        if (selectedIdx == -1) {
            int nextArrival = 1e9;
            for (size_t i = 0; i < total; ++i) {
                if (!isFinished[i] && processes[i].arrivalTime < nextArrival) {
                    nextArrival = processes[i].arrivalTime;
                }
            }
            currentTime = nextArrival;
            continue;
        }

        auto& p = processes[selectedIdx];
        p.status = 'R';
        for (int t = 0; t < p.burstTime; ++t) {
            timeline.push_back(p.name);
        }
        currentTime += p.burstTime;
        p.finishTime = currentTime;
        p.status = 'F';

        isFinished[selectedIdx] = true;
        finished.push_back(p);
        completedCount++;
    }

    calculateMetrics(finished);
    return buildResult("最高响应比优先算法 (HRRN)", finished, timeline);
}

// ====================================================================
//  5. 算法实现：动态优先级调度 (按时间片轮转，优先级递减)
// ====================================================================
std::vector<std::vector<PCB>> ProcessManager::runDynamicPriority(std::vector<PCB> processes, int timeSlice) {
    // 用于保存每一步（每一个时间片）所有进程的PCB状态快照，供图形化界面回放
    std::vector<std::vector<PCB>> history;
    
    int currentTime = 0;
    bool allDone = false;

    // 初始化运行状态
    for (auto& p : processes) {
        p.remainingTime = p.burstTime;
        p.cpuTime = 0;
        p.status = 'W';
    }

    while (!allDone) {
        // 找出所有已到达且未完成的进程中，优先级最高的那个
        int selectedIdx = -1;
        int maxPriority = -1e9;

        for (size_t i = 0; i < processes.size(); ++i) {
            if (processes[i].remainingTime > 0 && processes[i].arrivalTime <= currentTime) {
                if (processes[i].priority > maxPriority) {
                    maxPriority = processes[i].priority;
                    selectedIdx = i;
                } else if (processes[i].priority == maxPriority) {
                    if (processes[i].arrivalTime < processes[selectedIdx].arrivalTime) {
                        selectedIdx = i;
                    }
                }
            }
        }

        // 如果当前没有进程到达，时间推进
        if (selectedIdx == -1) {
            // 检查是否全部完成了
            bool hasRemaining = false;
            for (const auto& p : processes) {
                if (p.remainingTime > 0) {
                    hasRemaining = true;
                    break;
                }
            }
            if (!hasRemaining) {
                break; // 全部执行完毕
            }
            
            // 推进到最近的到达时间
            int nextArrival = 1e9;
            for (const auto& p : processes) {
                if (p.remainingTime > 0 && p.arrivalTime < nextArrival) {
                    nextArrival = p.arrivalTime;
                }
            }
            currentTime = nextArrival;
            continue;
        }

        // 执行选中的进程一个时间片
        auto& p = processes[selectedIdx];
        p.status = 'R';
        
        // 记录运行前状态快照
        history.push_back(processes);

        // 模拟运行一个时间片 (1个时间单位)
        p.remainingTime -= timeSlice;
        p.cpuTime += timeSlice;
        currentTime += timeSlice;

        if (p.remainingTime <= 0) {
            p.remainingTime = 0;
            p.status = 'F';
            p.finishTime = currentTime;
        } else {
            if(p.priority > 0){
                p.priority -= 1; // 优先级降低一级
            }
            p.status = 'W';  // 重新放回就绪队列
        }

        // 检查是否全部完成
        allDone = true;
        for (const auto& proc : processes) {
            if (proc.remainingTime > 0) {
                allDone = false;
                break;
            }
        }
    }

    // 压入最终完成状态快照
    history.push_back(processes);
    return history;
}

// ====================================================================
//  6. 生产者-消费者（多线程 + 真实信号量）
// ====================================================================

// 线程间共享状态
struct ProdConsShared {
    HANDLE hFull;                       // full 信号量
    HANDLE hEmpty;                      // empty 信号量
    CRITICAL_SECTION cs;                // 互斥锁（保护缓冲池）
    std::vector<std::string> pool;
    int maxCap;
    int curCount;                       // 实际产品数
    int mutexVal;                       // mutex 值（1=未锁, 0=已锁）
    volatile LONG totalOps;
    int maxSteps;                       // 最大操作数（超了线程退出）
    volatile bool running;              // 是否继续运行
    std::vector<ProdConsStep> history;
    CRITICAL_SECTION histCs;            // 保护 history 的锁
};

// 记录一步操作（线程安全）
static void recordStep(ProdConsShared* sh, const std::string& actor,
                       const std::string& op, const std::string& detail) {
    EnterCriticalSection(&sh->histCs);
    if (!sh->running || sh->totalOps >= sh->maxSteps) {
        LeaveCriticalSection(&sh->histCs);
        return;
    }
    ProdConsStep s;
    s.actor = actor; s.operation = op; s.detail = detail;
    s.semFull = sh->curCount;
    s.semEmpty = sh->maxCap - sh->curCount;
    s.semMutex = sh->mutexVal;
    s.productCount = sh->curCount;
    s.pool = sh->pool;
    sh->history.push_back(s);
    InterlockedIncrement(&sh->totalOps);
    LeaveCriticalSection(&sh->histCs);
}

DWORD WINAPI ProducerThread(LPVOID param) {
    ProdConsShared* sh = (ProdConsShared*)param;
    while (sh->running && sh->totalOps < sh->maxSteps) {
        // P(empty)
        DWORD dw = WaitForSingleObject(sh->hEmpty, 200);
        if (dw != WAIT_OBJECT_0) continue;
        if (!sh->running) { ReleaseSemaphore(sh->hEmpty, 1, NULL); break; }
        recordStep(sh, "生产者", "P(empty)",
                   "P(empty)：申请空缓冲区成功，empty--");

        // P(mutex)
        sh->mutexVal--;
        EnterCriticalSection(&sh->cs);
        recordStep(sh, "生产者", "P(mutex)",
                   "P(mutex)：进入临界区，mutex=0");

        // 生产
        sh->pool[sh->curCount] = "Product";
        sh->curCount++;
        recordStep(sh, "生产者", "生产",
                   "生产一个产品，放入缓冲块[" + std::to_string(sh->curCount - 1) + "]");

        // V(mutex)
        LeaveCriticalSection(&sh->cs);
        sh->mutexVal++;
        recordStep(sh, "生产者", "V(mutex)",
                   "V(mutex)：退出临界区，mutex=1");

        // V(full)
        ReleaseSemaphore(sh->hFull, 1, NULL);
        recordStep(sh, "生产者", "V(full)",
                   "V(full)：full++，可唤醒消费者");
    }
    return 0;
}

DWORD WINAPI ConsumerThread(LPVOID param) {
    ProdConsShared* sh = (ProdConsShared*)param;
    while (sh->running && sh->totalOps < sh->maxSteps) {
        // P(full)
        DWORD dw = WaitForSingleObject(sh->hFull, 200);
        if (dw != WAIT_OBJECT_0) continue;
        if (!sh->running) { ReleaseSemaphore(sh->hFull, 1, NULL); break; }
        recordStep(sh, "消费者", "P(full)",
                   "P(full)：申请满缓冲区成功，full--");

        // P(mutex)
        sh->mutexVal--;
        EnterCriticalSection(&sh->cs);
        recordStep(sh, "消费者", "P(mutex)",
                   "P(mutex)：进入临界区，mutex=0");

        // 消费
        sh->curCount--;
        sh->pool[sh->curCount] = "[Empty]";
        recordStep(sh, "消费者", "消费",
                   "消费一个产品，清空缓冲块[" + std::to_string(sh->curCount) + "]");

        // V(mutex)
        LeaveCriticalSection(&sh->cs);
        sh->mutexVal++;
        recordStep(sh, "消费者", "V(mutex)",
                   "V(mutex)：退出临界区，mutex=1");

        // V(empty)
        ReleaseSemaphore(sh->hEmpty, 1, NULL);
        recordStep(sh, "消费者", "V(empty)",
                   "V(empty)：empty++，可唤醒生产者");
    }
    return 0;
}

std::vector<ProdConsStep> ProcessManager::startProdConsThreads(int maxStep, int initCount, int maxCapacity) {
    ProdConsShared sh;
    sh.maxCap = maxCapacity;
    sh.curCount = initCount;
    sh.mutexVal = 1;
    sh.maxSteps = maxStep;
    sh.running = true;
    sh.totalOps = 0;
    sh.pool.assign(maxCapacity, "[Empty]");
    for (int i = 0; i < initCount; ++i) sh.pool[i] = "Product";

    // 创建 Windows 信号量
    sh.hFull  = CreateSemaphore(NULL, initCount, maxCapacity, NULL);
    sh.hEmpty = CreateSemaphore(NULL, maxCapacity - initCount, maxCapacity, NULL);
    InitializeCriticalSection(&sh.cs);
    InitializeCriticalSection(&sh.histCs);

    // 初始状态（计入历史用于展示，但不计入步数限制）
    recordStep(&sh, "系统", "初始化",
               "缓冲池初始化，初始商品 " + std::to_string(initCount) +
               " 个，空闲 " + std::to_string(maxCapacity - initCount) + " 个");
    sh.totalOps = 0;

    // 创建线程
    DWORD tid1, tid2;
    HANDLE hProd = CreateThread(NULL, 0, ProducerThread, &sh, 0, &tid1);
    HANDLE hCons = CreateThread(NULL, 0, ConsumerThread, &sh, 0, &tid2);

    // 等待达到步数上限（额外留 5 步缓冲，保证最后一个周期完整）
    while (sh.totalOps < maxStep) Sleep(50);
    Sleep(100);  // 等正在执行的周期完成

    // 停止线程
    sh.running = false;
    WaitForSingleObject(hProd, 1000);
    WaitForSingleObject(hCons, 1000);

    // 清理
    CloseHandle(hProd); CloseHandle(hCons);
    CloseHandle(sh.hFull); CloseHandle(sh.hEmpty);
    DeleteCriticalSection(&sh.cs);
    DeleteCriticalSection(&sh.histCs);

    return sh.history;
}
