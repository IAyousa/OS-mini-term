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
    std::string actionDesc;       // 事件描述 ("生产者生产了一个产品", "缓冲池满...", "消费者取出了一个产品"等)
    int productCount = 0;         // 当前产品总数
    std::vector<std::string> pool; // 缓冲池状态可视化：保存如 "Product", "[Empty]" 的字符串
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

    // --- 生产者-消费者模拟 (步进式) ---
    // 模拟产生包含生产者和消费者行为的步进历史轨迹（方便图形化步进演示）
    static std::vector<ProdConsStep> generateProdConsHistory(int stepCount = 20, int initCount = 5, int maxCapacity = 20);
};
