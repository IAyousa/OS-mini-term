#pragma once
#include <string>
#include <vector>

// ====================================================================
//  1. 进程控制块 (PCB) 结构体定义
// ====================================================================
struct PCB {
    std::string name;       // 进程名
    int priority = 0;       // 优先级 / 优先权数
    int arrivalTime = 0;    // 到达时间
    int burstTime = 0;      // 需要运行时间 (服务时间)
    int remainingTime = 0;  // 剩余运行时间
    int cpuTime = 0;        // 已占用 CPU 时间
    char status = 'W';      // 状态: 'W' (Wait/就绪), 'R' (Run/运行), 'F' (Finish/完成)
    
    // 调度计算所得指标
    int finishTime = 0;     // 完成时刻
    int turnAroundTime = 0; // 周转时间
    double weightedTurnAroundTime = 0.0; // 带权周转时间
};

// ====================================================================
//  2. 生产者 - 消费者步进状态记录
// ====================================================================
struct ProdConsStep {
    std::string actor;              // "生产者" / "消费者" / "系统"
    std::string operation;          // "P(empty)" / "V(empty)" / "P(full)" / "V(full)" / "P(mutex)" / "V(mutex)" / "生产" / "消费" / "阻塞"
    std::string detail;             // 详细描述
    int productCount = 0;           // 当前产品总数
    int semFull = 0;                // full 信号量值
    int semEmpty = 0;               // empty 信号量值
    int semMutex = 1;               // mutex 信号量值
    std::vector<std::string> pool;  // 缓冲池状态
};

// ====================================================================
//  3. 进程调度核心管理类声明
// ====================================================================
struct SchedulingResult {
    std::string algoName;
    std::vector<PCB> finishedProcesses; // 已按完成顺序排列的进程列表
    double avgTurnAroundTime = 0.0;     // 平均周转时间
    double avgWeightedTurnAroundTime = 0.0; // 平均带权周转时间
    std::vector<std::string> executionTimeline; // 运行轨迹甘特图序列
};

class ProcessManager {
public:
    // --- 进程调度算法 ---
    // 先来先服务 (FCFS)
    static SchedulingResult runFCFS(std::vector<PCB> processes);
    
    // 短作业优先 (SJF) - 非抢占式
    static SchedulingResult runSJF(std::vector<PCB> processes);
    
    // 高响应比优先 (HRRN)
    static SchedulingResult runHRRN(std::vector<PCB> processes);

    // 动态优先级调度算法（时间片轮转，优先级递减）
    // 按时间片分配 CPU，一个时间片后，运行进程已用CPU时间+1，优先级减1，重新排队
    static std::vector<std::vector<PCB>> runDynamicPriority(std::vector<PCB> processes, int timeSlice = 1);

    // --- 生产者-消费者模拟 (多线程 + 真实信号量) ---
    // 使用 Windows 线程 + 信号量对象实现真正的并发 PV 操作
    static std::vector<ProdConsStep> startProdConsThreads(int maxStep, int initCount, int maxCapacity);
};
