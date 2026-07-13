#include "../include/memory_manager.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>

MemoryManager::MemoryManager(int frameCount, int pageSizeKB)
    : frameCount_(frameCount), pageSizeBytes_(pageSizeKB * 1024) {}

std::vector<int> MemoryManager::generateAddressStream(int count) {
    std::vector<int> addresses;
    addresses.reserve(count);
    
    // 任务书要求：
    // 50%的指令是顺序执行的
    // 25%的指令均匀地散布在前地址部分
    // 25%的地址是均匀地散布在后地址部分
    int current = rand() % 320; // 假设指令地址范围为 [0, 319]
    
    for (int i = 0; i < count; ) {
        // 顺序执行 (50%)
        if (i < count) {
            addresses.push_back(current);
            current = (current + 1) % 320;
            i++;
        }
        if (i < count) {
            addresses.push_back(current);
            current = (current + 1) % 320;
            i++;
        }
        
        // 前地址部分 (25%)
        if (i < count && current > 0) {
            int prev = rand() % current;
            addresses.push_back(prev);
            current = prev;
            i++;
        }
        
        // 后地址部分 (25%)
        if (i < count && current < 319) {
            int next = current + 1 + rand() % (320 - (current + 1));
            addresses.push_back(next);
            current = next;
            i++;
        }
    }
    return addresses;
}

MemoryResult MemoryManager::runFIFO(const std::vector<int>& addresses) {
    MemoryResult result;
    result.algorithmName = "先进先出淘汰算法 (FIFO)";
    result.totalRequests = addresses.size();
    
    std::vector<int> frames(frameCount_, -1); // 内存物理块，初始化为 -1（空）
    std::vector<int> enterTime(frameCount_, -1); // 记录每个物理块被装入的时间
    int timeCounter = 0;
    
    for (int addr : addresses) {
        int pageNum = addr / 10; // 假定每页 10 个指令/地址，共 32 页
        
        bool isHit = false;
        int emptyIdx = -1;
        
        // 检查是否命中，以及寻找空闲块
        for (int i = 0; i < frameCount_; ++i) {
            if (frames[i] == pageNum) {
                isHit = true;
                break;
            }
            if (frames[i] == -1 && emptyIdx == -1) {
                emptyIdx = i;
            }
        }
        
        MemoryStep step;
        step.visitAddress = addr;
        step.pageNumber = pageNum;
        step.isHit = isHit;
        
        if (isHit) {
            step.memoryFrames = frames;
        } else {
            result.pageFaults++;
            if (emptyIdx != -1) {
                // 有空闲块
                frames[emptyIdx] = pageNum;
                enterTime[emptyIdx] = timeCounter++;
            } else {
                // 满了，执行 FIFO 淘汰
                int oldestIdx = 0;
                int minTime = enterTime[0];
                for (int i = 1; i < frameCount_; ++i) {
                    if (enterTime[i] < minTime) {
                        minTime = enterTime[i];
                        oldestIdx = i;
                    }
                }
                frames[oldestIdx] = pageNum;
                enterTime[oldestIdx] = timeCounter++;
            }
            step.memoryFrames = frames;
        }
        result.steps.push_back(step);
    }
    
    result.faultRate = (double)result.pageFaults / result.totalRequests;
    return result;
}

MemoryResult MemoryManager::runLRU(const std::vector<int>& addresses) {
    MemoryResult result;
    result.algorithmName = "最近最少使用淘汰算法 (LRU)";
    result.totalRequests = addresses.size();
    
    std::vector<int> frames(frameCount_, -1);
    std::vector<int> lastUsedTime(frameCount_, -1); // 记录每个物理块最近一次使用的时间
    int timeCounter = 0;
    
    for (int addr : addresses) {
        int pageNum = addr / 10;
        
        bool isHit = false;
        int emptyIdx = -1;
        
        for (int i = 0; i < frameCount_; ++i) {
            if (frames[i] == pageNum) {
                isHit = true;
                lastUsedTime[i] = timeCounter++; // 命中，更新访问时间
                break;
            }
            if (frames[i] == -1 && emptyIdx == -1) {
                emptyIdx = i;
            }
        }
        
        MemoryStep step;
        step.visitAddress = addr;
        step.pageNumber = pageNum;
        step.isHit = isHit;
        
        if (isHit) {
            step.memoryFrames = frames;
        } else {
            result.pageFaults++;
            if (emptyIdx != -1) {
                frames[emptyIdx] = pageNum;
                lastUsedTime[emptyIdx] = timeCounter++;
            } else {
                // 淘汰最久未使用的
                int lruIdx = 0;
                int minTime = lastUsedTime[0];
                for (int i = 1; i < frameCount_; ++i) {
                    if (lastUsedTime[i] < minTime) {
                        minTime = lastUsedTime[i];
                        lruIdx = i;
                    }
                }
                frames[lruIdx] = pageNum;
                lastUsedTime[lruIdx] = timeCounter++;
            }
            step.memoryFrames = frames;
        }
        result.steps.push_back(step);
    }
    
    result.faultRate = (double)result.pageFaults / result.totalRequests;
    return result;
}
