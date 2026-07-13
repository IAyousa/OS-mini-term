#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>

// 磁盘调度算法枚举
enum class DiskAlgorithm {
    FCFS,
    SSTF,
    SCAN,
    CSCAN
};

// 调度结果结构体
struct ScheduleResult {
    std::vector<int> sequence;   // 访问顺序（包含初始位置）
    int total_seek = 0;
    double avg_seek = 0.0;
    std::string algo_name;
    std::string algo_short_name;
    
    // 清除数据
    void clear() {
        sequence.clear();
        total_seek = 0;
        avg_seek = 0.0;
    }
};

class DiskScheduler {
public:
    // 构造函数
    DiskScheduler(const std::vector<int>& requests, int init_head);
    
    // 执行指定算法
    ScheduleResult run(DiskAlgorithm algo);
    
    // 获取数据
    std::vector<int> getRequests() const { return requests_; }
    int getInitHead() const { return init_head_; }
    
    // 获取最大磁道号（用于图形坐标）
    int getMaxTrack() const { return max_track_; }
    void setMaxTrack(int max_track) { max_track_ = max_track; }
    
    // 控制台打印结果
    void printResult(const ScheduleResult& result);
    
private:
    std::vector<int> requests_;   // 磁道请求序列
    int init_head_;               // 初始磁头位置
    int max_track_ = 199;         // 最大磁道号（默认199）
    
    // 各算法实现
    ScheduleResult fcfs();
    ScheduleResult sstf();
    ScheduleResult scan();
    ScheduleResult cscan();
};
