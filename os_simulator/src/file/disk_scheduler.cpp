#include "../../include/disk_scheduler.h"
#include <iostream>
#include <iomanip>
#include <climits>

DiskScheduler::DiskScheduler(const std::vector<int>& requests, int init_head)
    : requests_(requests), init_head_(init_head) {}

ScheduleResult DiskScheduler::run(DiskAlgorithm algo) {
    switch (algo) {
        case DiskAlgorithm::FCFS:  return fcfs();
        case DiskAlgorithm::SSTF:  return sstf();
        case DiskAlgorithm::SCAN:  return scan();
        case DiskAlgorithm::CSCAN: return cscan();
        default: return fcfs();
    }
}

// 1. 先来先服务
ScheduleResult DiskScheduler::fcfs() {
    ScheduleResult result;
    result.algo_name = "先来先服务";
    result.algo_short_name = "FCFS";
    
    int current = init_head_;
    result.sequence.push_back(current);
    result.total_seek = 0;
    
    for (int req : requests_) {
        result.sequence.push_back(req);
        result.total_seek += std::abs(current - req);
        current = req;
    }
    
    result.avg_seek = (double)result.total_seek / requests_.size();
    return result;
}

// 2. 最短寻道时间优先
ScheduleResult DiskScheduler::sstf() {
    ScheduleResult result;
    result.algo_name = "最短寻道时间优先";
    result.algo_short_name = "SSTF";
    
    std::vector<int> remaining = requests_;
    int current = init_head_;
    result.sequence.push_back(current);
    result.total_seek = 0;
    
    while (!remaining.empty()) {
        int min_dist = INT_MAX;
        int min_idx = 0;
        
        for (size_t i = 0; i < remaining.size(); ++i) {
            int dist = std::abs(current - remaining[i]);
            if (dist < min_dist) {
                min_dist = dist;
                min_idx = i;
            }
        }
        
        int next = remaining[min_idx];
        result.sequence.push_back(next);
        result.total_seek += min_dist;
        current = next;
        remaining.erase(remaining.begin() + min_idx);
    }
    
    result.avg_seek = (double)result.total_seek / requests_.size();
    return result;
}

// 3. 扫描算法（电梯算法）
ScheduleResult DiskScheduler::scan() {
    ScheduleResult result;
    result.algo_name = "扫描算法（电梯）";
    result.algo_short_name = "SCAN";
    
    std::vector<int> sorted = requests_;
    std::sort(sorted.begin(), sorted.end());
    
    int current = init_head_;
    result.sequence.push_back(current);
    result.total_seek = 0;
    
    std::vector<int> left, right;
    for (int req : sorted) {
        if (req < current) left.push_back(req);
        else right.push_back(req);
    }
    
    // 先向外移动
    for (int req : right) {
        result.total_seek += std::abs(current - req);
        current = req;
        result.sequence.push_back(req);
    }
    
    // 再向内移动
    for (auto it = left.rbegin(); it != left.rend(); ++it) {
        result.total_seek += std::abs(current - *it);
        current = *it;
        result.sequence.push_back(*it);
    }
    
    result.avg_seek = (double)result.total_seek / requests_.size();
    return result;
}

// 4. 循环扫描算法
ScheduleResult DiskScheduler::cscan() {
    ScheduleResult result;
    result.algo_name = "循环扫描算法";
    result.algo_short_name = "CSCAN";
    
    std::vector<int> sorted = requests_;
    std::sort(sorted.begin(), sorted.end());
    
    int current = init_head_;
    result.sequence.push_back(current);
    result.total_seek = 0;
    
    std::vector<int> left, right;
    for (int req : sorted) {
        if (req < current) left.push_back(req);
        else right.push_back(req);
    }
    
    // 向外移动
    for (int req : right) {
        result.total_seek += std::abs(current - req);
        current = req;
        result.sequence.push_back(req);
    }
    
    // 跳转到最内侧
    if (!left.empty()) {
        result.total_seek += std::abs(current - max_track_) + max_track_ - left[0];
        current = left[0];
        result.sequence.push_back(current);
        
        for (size_t i = 1; i < left.size(); ++i) {
            result.total_seek += std::abs(current - left[i]);
            current = left[i];
            result.sequence.push_back(current);
        }
    }
    
    result.avg_seek = (double)result.total_seek / requests_.size();
    return result;
}

// 控制台打印
void DiskScheduler::printResult(const ScheduleResult& result) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "磁盘调度算法: " << result.algo_name << " (" << result.algo_short_name << ")" << std::endl;
    std::cout << "访问顺序: ";
    for (size_t i = 0; i < result.sequence.size(); ++i) {
        std::cout << result.sequence[i];
        if (i < result.sequence.size() - 1) std::cout << " -> ";
    }
    std::cout << std::endl;
    std::cout << "总寻道长度: " << result.total_seek << std::endl;
    std::cout << "平均寻道长度: " << std::fixed << std::setprecision(2) << result.avg_seek << std::endl;
    std::cout << "========================================" << std::endl;
}