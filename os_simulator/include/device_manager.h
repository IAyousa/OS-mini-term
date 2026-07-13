#pragma once
#include <vector>
#include <string>

// ============================================================
//  银行家算法数据结构
// ============================================================

struct BankerSystem {
    int processCount;       // 进程数 M
    int resourceCount;      // 资源种类数 N

    std::vector<std::vector<int>> max;          // MAX[M][N]  最大需求
    std::vector<std::vector<int>> allocation;   // ALLOCATION[M][N]  已分配
    std::vector<std::vector<int>> need;         // NEED[M][N]  还需资源
    std::vector<int> available;                 // AVAILABLE[N]  系统可用

    // 当前请求 (由用户输入)
    int requestProcess;             // 发起请求的进程号
    std::vector<int> request;       // Request[N] 请求向量

    // 执行结果
    bool lastRequestGranted;        // 上次请求是否被批准
    std::string lastMessage;        // 上次操作的描述信息
};

// ============================================================
//  函数声明
// ============================================================

// 初始化系统。不带 M,N 参数时使用默认示例数据 (M=5, N=3, 任务书数据)
// 指定 M,N 时按自定义规模清零初始化
void initSystem(BankerSystem& sys, int M = 5, int N = 3);

// 安全检查：判断当前状态是否安全
// 返回 true=安全, false=不安全
// safeSequence 输出安全序列
bool isSafeState(const BankerSystem& sys, std::vector<int>& safeSequence);

// 尝试分配：检查 Request 是否合法且安全
// 如果安全则实际分配资源并返回 true，否则回滚返回 false
bool tryAllocate(BankerSystem& sys, int processIdx, const std::vector<int>& request);
