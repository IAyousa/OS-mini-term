#pragma once
#include <vector>
#include <string>

// 存储页面状态的结构体
struct PageState {
    int pageNumber = -1;      // 页号，-1 表示空闲
    bool loaded = false;      // 是否在内存中
};

// 页面替换执行结果记录
struct MemoryStep {
    int visitAddress;              // 访问的指令逻辑地址
    int pageNumber;                // 对应的页号
    bool isHit;                    // 是否命中
    std::vector<int> memoryFrames; // 内存中各物理块当前的页面内容（-1 表示空
};

struct MemoryResult {
    std::string algorithmName;     // 算法名称
    int totalRequests = 0;         // 总访问次数
    int pageFaults = 0;            // 缺页次数
    double faultRate = 0.0;        // 缺页率
    std::vector<MemoryStep> steps; // 详细访问步骤历史
};

// 内存管理器类
class MemoryManager {
public:
    // 构造函数：指定物理块数（页表长度）和页面大小（KB，默认1KB）
    MemoryManager(int frameCount, int pageSizeKB = 1);

    // 运行先进先出 (FIFO) 算法
    MemoryResult runFIFO(const std::vector<int>& addresses);

    // 运行最近最少使用 (LRU) 算法
    MemoryResult runLRU(const std::vector<int>& addresses);

    // 辅助：生成任务书要求的随机指令地址流
    static std::vector<int> generateAddressStream(int count);

private:
    int frameCount_;
    int pageSizeBytes_;
};
