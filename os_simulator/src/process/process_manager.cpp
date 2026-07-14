#include "../include/process_manager.h"
#include <algorithm>
#include <cstdlib>
#include <queue>
#include <sstream>
#include <iomanip>

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
//  6. 生产者-消费者历史序列生成器
// ====================================================================
std::vector<ProdConsStep> ProcessManager::generateProdConsHistory(int stepCount, int initCount, int maxCapacity) {
    std::vector<ProdConsStep> history;
    int currentCount = initCount;

    // 初始化缓冲池
    std::vector<std::string> currentPool(maxCapacity, "[Empty]");
    for (int i = 0; i < currentCount; ++i) {
        currentPool[i] = "Product";
    }

    ProdConsStep initialStep;
    initialStep.actionDesc = "初始化缓冲池，初始商品数量: " + std::to_string(currentCount);
    initialStep.productCount = currentCount;
    initialStep.pool = currentPool;
    history.push_back(initialStep);

    for (int i = 0; i < stepCount; ++i) {
        ProdConsStep step;
        // 随机：0 代表生产者生产，1 代表消费者消费
        int r = rand() % 2;

        if (r == 0) {
            // 生产者行为
            if (currentCount >= maxCapacity) {
                step.actionDesc = "生产了一个新产品，但缓冲池已满！进入阻塞等待...";
            } else {
                currentPool[currentCount] = "Product";
                currentCount++;
                step.actionDesc = "生产了一个新产品！成功存入缓冲池中";
            }
        } else {
            // 消费者行为
            if (currentCount <= 0) {
                step.actionDesc = "尝试取出产品，但缓冲池已空！进入阻塞等待...";
            } else {
                currentCount--;
                currentPool[currentCount] = "[Empty]";
                step.actionDesc = "取出了一个产品！消费成功";
            }
        }

        step.productCount = currentCount;
        step.pool = currentPool;
        history.push_back(step);
    }

    return history;
}
