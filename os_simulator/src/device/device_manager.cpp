#include "device_manager.h"
#include <cstdlib>
#include <cstring>

// ============================================================
//  初始化系统
// ============================================================
void initSystem(BankerSystem& sys, int M, int N) {
    sys.processCount = M;
    sys.resourceCount = N;

    // 调整所有矩阵大小（用赋值构造强制清零，resize 在尺寸不变时不会重置数据）
    sys.max = std::vector<std::vector<int>>(M, std::vector<int>(N, 0));
    sys.allocation = std::vector<std::vector<int>>(M, std::vector<int>(N, 0));
    sys.need = std::vector<std::vector<int>>(M, std::vector<int>(N, 0));
    sys.available = std::vector<int>(N, 0);

    sys.request.resize(N, 0);
    sys.requestProcess = 0;
    sys.lastRequestGranted = false;
    sys.lastMessage = "";

    // M=5, N=3 时填入任务书示例数据
    if (M == 5 && N == 3) {
        // MAX[M][N] — 最大需求
        int defaultMax[5][3] = {
            {7, 5, 3},
            {3, 2, 2},
            {9, 0, 2},
            {2, 2, 2},
            {4, 3, 3}
        };
        for (int i = 0; i < M; ++i)
            for (int j = 0; j < N; ++j)
                sys.max[i][j] = defaultMax[i][j];

        // AVAILABLE[N] — 系统可用资源
        int defaultAvail[3] = {10, 5, 7};
        for (int j = 0; j < N; ++j)
            sys.available[j] = defaultAvail[j];

        // 初始 ALLOCATION 全 0，所以 NEED = MAX
        for (int i = 0; i < M; ++i)
            for (int j = 0; j < N; ++j)
                sys.need[i][j] = sys.max[i][j];

        sys.lastMessage = "系统已初始化：5进程，3类资源，使用示例数据";
    } else {
        // 任意 M,N：生成合理的随机初始数据
        // Available: 每类资源量 = 5 * (M/2 + 1) + j*2
        for (int j = 0; j < N; ++j)
            sys.available[j] = 5 * (M / 2 + 1) + j * 2;

        // MAX: 每个进程对每类资源的最大需求
        for (int i = 0; i < M; ++i)
            for (int j = 0; j < N; ++j) {
                int maxVal = sys.available[j] * 3 / 4;
                sys.max[i][j] = (maxVal > 0) ? (rand() % maxVal + 1) : 1;
            }

        // NEED = MAX（初始分配全 0）
        for (int i = 0; i < M; ++i)
            for (int j = 0; j < N; ++j)
                sys.need[i][j] = sys.max[i][j];

        sys.lastMessage = "系统已初始化：规模 " + std::to_string(M) + "×" + std::to_string(N) + "，自动生成初始数据";
    }
}

// ============================================================
//  安全检查
//  判断当前状态是否安全，若安全则通过 safeSequence 输出安全序列
// ============================================================
bool isSafeState(const BankerSystem& sys, std::vector<int>& safeSequence) {
    int M = sys.processCount;
    int N = sys.resourceCount;

    // 工作向量 Work = Available（拷贝一份）
    std::vector<int> work = sys.available;

    // Finish[M] — 标记进程是否已执行完毕
    std::vector<bool> finish(M, false);

    safeSequence.clear();

    // 最多尝试 M 轮（每轮找一个能完成的进程）
    for (int round = 0; round < M; ++round) {
        bool found = false;

        for (int i = 0; i < M; ++i) {
            if (finish[i]) continue;

            // 检查 Need[i] <= Work ?
            bool canAllocate = true;
            for (int j = 0; j < N; ++j) {
                if (sys.need[i][j] > work[j]) {
                    canAllocate = false;
                    break;
                }
            }

            if (canAllocate) {
                // 假设进程 i 执行完毕，释放资源
                for (int j = 0; j < N; ++j)
                    work[j] += sys.allocation[i][j];

                finish[i] = true;
                safeSequence.push_back(i);
                found = true;
                // 从头开始扫描（因为释放资源后可能有其他进程满足条件）
                break;
            }
        }

        if (!found) break;  // 这一轮没找到可执行的进程 → 不安全
    }

    // 所有进程都完成了 → 安全状态
    for (int i = 0; i < M; ++i)
        if (!finish[i]) return false;

    return true;
}

// ============================================================
//  尝试分配资源
//  流程：合法性检查 → 试分配 → 安全检查 → 确认/回滚
// ============================================================
bool tryAllocate(BankerSystem& sys, int processIdx, const std::vector<int>& request) {
    int M = sys.processCount;
    int N = sys.resourceCount;

    // ---- 1. 合法性检查 ----
    // Request <= Need ?
    for (int j = 0; j < N; ++j) {
        if (request[j] > sys.need[processIdx][j]) {
            sys.lastRequestGranted = false;
            sys.lastMessage = "进程 P" + std::to_string(processIdx) +
                              " 请求资源超出其最大需求 (Request > Need)，拒绝！";
            return false;
        }
    }

    // Request <= Available ?
    for (int j = 0; j < N; ++j) {
        if (request[j] > sys.available[j]) {
            sys.lastRequestGranted = false;
            sys.lastMessage = "进程 P" + std::to_string(processIdx) +
                              " 请求资源超出系统可用量 (Request > Available)，阻塞等待！";
            return false;
        }
    }

    // ---- 2. 试分配 ----
    // Available -= Request
    for (int j = 0; j < N; ++j)
        sys.available[j] -= request[j];

    // Allocation[i] += Request
    for (int j = 0; j < N; ++j)
        sys.allocation[processIdx][j] += request[j];

    // Need[i] -= Request
    for (int j = 0; j < N; ++j)
        sys.need[processIdx][j] -= request[j];

    // ---- 3. 安全性检查 ----
    std::vector<int> safeSeq;
    bool safe = isSafeState(sys, safeSeq);

    if (safe) {
        // 安全 → 确认分配
        sys.lastRequestGranted = true;
        sys.lastMessage = "分配成功！安全序列: P";
        for (size_t i = 0; i < safeSeq.size(); ++i) {
            if (i > 0) sys.lastMessage += " → P";
            sys.lastMessage += std::to_string(safeSeq[i]);
        }
        return true;
    } else {
        // 不安全 → 回滚
        for (int j = 0; j < N; ++j)
            sys.available[j] += request[j];
        for (int j = 0; j < N; ++j)
            sys.allocation[processIdx][j] -= request[j];
        for (int j = 0; j < N; ++j)
            sys.need[processIdx][j] += request[j];

        sys.lastRequestGranted = false;
        sys.lastMessage = "分配将导致系统进入不安全状态，拒绝分配！";
        return false;
    }
}
