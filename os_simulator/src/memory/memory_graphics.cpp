#include <graphics.h>
#include <conio.h>  // 引入 Windows 控制台输入输出头文件以支持 kbhit()
#include "../../include/memory_manager.h"
#include "../../include/graphics_utils.h"
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>

// ============================================================
//  存储管理图形界面绘制与子菜单
// ============================================================

static void drawMemoryTable(const MemoryResult& res, int currentStepIdx) {
    clearWhite();

    // 标题
    std::string title = "存储管理虚拟页淘汰可视化 - " + res.algorithmName;
    int tx = textWidth(title, 24);
    drawText((W_WIDTH - tx) / 2, 20, title, 24, BLACK);

    // 顶部统计面板
    setfillcolor(RGB(245, 245, 245));
    setlinecolor(RGB(180, 180, 180));
    setlinestyle(PS_SOLID, 1);
    fillrectangle(40, 60, W_WIDTH - 40, 110);
    rectangle(40, 60, W_WIDTH - 40, 110);

    std::stringstream ss;
    ss << "总访问次数: " << res.totalRequests << "   "
       << "缺页次数: " << res.pageFaults << "   "
       << "缺页率: " << std::fixed << std::setprecision(2) << (res.faultRate * 100.0) << "%";
    drawText(60, 75, ss.str(), 16, RGB(30, 30, 30));

    // 当前步骤说明
    if (currentStepIdx >= 0 && currentStepIdx < (int)res.steps.size()) {
        const auto& step = res.steps[currentStepIdx];
        std::stringstream ss2;
        ss2 << "当前步骤: " << (currentStepIdx + 1) << " / " << res.totalRequests
            << " | 访问指令物理/逻辑地址: " << step.visitAddress
            << " | 对应页号: " << step.pageNumber
            << " | 状态: " << (step.isHit ? "命中 (Hit)" : "缺页中断 (Page Fault)");
        COLORREF statusColor = step.isHit ? RGB(70, 150, 70) : RGB(200, 60, 60);
        drawText(40, 130, ss2.str(), 16, statusColor);
    }

    // 绘制虚拟页表和物理内存块状态
    int startY = 180;
    int blockW = 100;
    int blockH = 40;

    // 绘制当前内存物理块中的页
    drawText(40, startY, "当前物理内存块 (Frames) 状态:", 18, BLACK);
    if (currentStepIdx >= 0 && currentStepIdx < (int)res.steps.size()) {
        const auto& step = res.steps[currentStepIdx];
        int startX = 40;
        for (size_t i = 0; i < step.memoryFrames.size(); ++i) {
            int page = step.memoryFrames[i];
            
            // 绘制物理框
            setlinecolor(BLACK);
            setlinestyle(PS_SOLID, 2);
            if (page == -1) {
                setfillcolor(RGB(240, 240, 240));
                fillrectangle(startX, startY + 40, startX + blockW, startY + 40 + blockH);
                drawText(startX + 15, startY + 50, "Empty", 16, RGB(150, 150, 150));
            } else {
                // 如果刚刚调入或命中，用不同颜色高亮
                setfillcolor(step.isHit ? RGB(220, 245, 220) : RGB(255, 220, 220));
                fillrectangle(startX, startY + 40, startX + blockW, startY + 40 + blockH);
                std::string pageStr = "Page " + std::to_string(page);
                drawText(startX + 20, startY + 50, pageStr, 16, BLACK);
            }
            
            std::string frameLabel = "块 " + std::to_string(i);
            drawText(startX + 25, startY + 40 + blockH + 10, frameLabel, 14, RGB(100, 100, 100));
            startX += blockW + 30;
        }
    }

    // 绘制历史访问轨迹流水表
    int historyY = startY + 140;
    drawText(40, historyY, "历史页面调入及淘汰流水表 (最新 8 步):", 18, BLACK);
    
    int cellW = 100;
    int cellH = 30;
    int headX = 40;
    
    // 表头
    setlinecolor(RGB(200, 200, 200));
    setlinestyle(PS_SOLID, 1);
    rectangle(headX, historyY + 30, headX + cellW, historyY + 30 + cellH);
    drawText(headX + 10, historyY + 38, "访问地址", 14, BLACK);
    
    rectangle(headX, historyY + 30 + cellH, headX + cellW, historyY + 30 + cellH * 2);
    drawText(headX + 10, historyY + 38 + cellH, "对应页号", 14, BLACK);

    rectangle(headX, historyY + 30 + cellH * 2, headX + cellW, historyY + 30 + cellH * 3);
    drawText(headX + 10, historyY + 38 + cellH * 2, "是否命中", 14, BLACK);

    // 确定展示的步骤区间 (显示当前步骤及前 7 步)
    int showCount = 8;
    int startStep = std::max(0, currentStepIdx - showCount + 1);
    int endStep = currentStepIdx;

    int colX = headX + cellW;
    for (int s = startStep; s <= endStep; ++s) {
        const auto& step = res.steps[s];
        
        // 访问地址
        setfillcolor(WHITE);
        fillrectangle(colX, historyY + 30, colX + cellW, historyY + 30 + cellH);
        rectangle(colX, historyY + 30, colX + cellW, historyY + 30 + cellH);
        drawText(colX + 30, historyY + 38, std::to_string(step.visitAddress), 14, BLACK);

        // 对应页号
        fillrectangle(colX, historyY + 30 + cellH, colX + cellW, historyY + 30 + cellH * 2);
        rectangle(colX, historyY + 30 + cellH, colX + cellW, historyY + 30 + cellH * 2);
        drawText(colX + 35, historyY + 38 + cellH, std::to_string(step.pageNumber), 14, BLACK);

        // 是否命中
        COLORREF hitBg = step.isHit ? RGB(220, 255, 220) : RGB(255, 220, 220);
        setfillcolor(hitBg);
        fillrectangle(colX, historyY + 30 + cellH * 2, colX + cellW, historyY + 30 + cellH * 3);
        rectangle(colX, historyY + 30 + cellH * 2, colX + cellW, historyY + 30 + cellH * 3);
        std::string hitStr = step.isHit ? "M (命中)" : "F (缺页)";
        COLORREF hitTextCol = step.isHit ? RGB(0, 120, 0) : RGB(200, 0, 0);
        drawText(colX + 15, historyY + 38 + cellH * 2, hitStr, 14, hitTextCol);

        colX += cellW;
    }

    // 底部控制提示
    std::string tipStr = "按 【左/右方向键】 或 【A/D键】 单数步步进/回退 | 按 【Enter】 自动演示 | 按 【ESC】 返回";
    int tipW = textWidth(tipStr, 16);
    drawText((W_WIDTH - tipW) / 2, W_HEIGHT - 60, tipStr, 16, RGB(100, 100, 100));
}

static void startMemorySimulation(MemoryResult res) {
    int currentStep = 0;
    int totalSteps = res.steps.size();
    bool autoPlay = false;

    while (true) {
        drawMemoryTable(res, currentStep);

        if (autoPlay) {
            Sleep(600);
            if (currentStep < totalSteps - 1) {
                currentStep++;
            } else {
                autoPlay = false;
            }
            
            // 检查是否有按键打断自动播放
            if (kbhit()) {
                ExMessage msg;
                getmessage(&msg, EM_KEY);
                if (msg.message == WM_KEYDOWN) {
                    autoPlay = false;
                    if (msg.vkcode == VK_ESCAPE) break;
                }
            }
            continue;
        }

        int key = waitForKey();
        if (key == VK_ESCAPE) {
            break;
        } else if (key == VK_RIGHT || key == 'D' || key == 'd') {
            if (currentStep < totalSteps - 1) currentStep++;
        } else if (key == VK_LEFT || key == 'A' || key == 'a') {
            if (currentStep > 0) currentStep--;
        } else if (key == VK_RETURN) {
            autoPlay = true;
            if (currentStep == totalSteps - 1) currentStep = 0; // 重头播放
        }
    }
}

// ============================================================
//  存储管理二级主菜单入口
// ============================================================
void memoryManagerSubMenu() {
    // 默认初始化：3个物理块，指令地址流20个
    int frameCount = 3;
    MemoryManager manager(frameCount, 1);
    std::vector<int> addressStream = MemoryManager::generateAddressStream(20);

    int selected = 0;
    const int MENU_COUNT = 5;
    
    while (true) {
        clearWhite();

        // 标题与设计参数
        int tx = textWidth("存储管理模拟子系统", 28);
        drawText((W_WIDTH - tx) / 2, 40, "存储管理模拟子系统", 28, BLACK);

        std::stringstream paramSS;
        paramSS << "当前系统配置: 内存物理块数 = " << frameCount << " | 页面尺寸 = 1 KB (10条指令/页)";
        tx = textWidth(paramSS.str(), 16);
        drawText((W_WIDTH - tx) / 2, 90, paramSS.str(), 16, RGB(100, 100, 100));

        // 绘制分割线
        setlinecolor(RGB(200, 200, 200));
        line(150, 120, W_WIDTH - 150, 120);

        // 菜单项数据
        struct MemoryMenuItem {
            std::string label;
            std::string desc;
        } items[MENU_COUNT] = {
            {"1. 生成/查看当前随机指令地址流", "生成长度为 20 的随机地址序列（50%顺序，25%前散，25%后散）"},
            {"2. 先进先出淘汰算法 (FIFO) 模拟", "按最先调入内存的页面予以淘汰的原则进行模拟，并计算缺页率"},
            {"3. 最近最少使用淘汰算法 (LRU) 模拟", "淘汰最近最长时间未被访问的页面，并计算缺页率"},
            {"4. 修改系统参数 (内存物理块数)", "修改模拟物理内存块的数量（范围 3 - 7 块）"},
            {"0. 返回主菜单", ""}
        };

        // 绘制菜单
        int startY = 160;
        for (int i = 0; i < MENU_COUNT; ++i) {
            bool isSelected = (i == selected);
            
            if (isSelected) {
                setfillcolor(RGB(230, 240, 255));
                setlinecolor(RGB(230, 240, 255));
                fillrectangle(100, startY - 5, W_WIDTH - 100, startY + 38);
            }

            // 标题及说明
            drawText(130, startY, items[i].label, 20, BLACK);
            if (!items[i].desc.empty()) {
                drawText(130, startY + 22, items[i].desc, 13, RGB(130, 130, 130));
            }

            startY += 60;
        }

        // 底部提示
        std::string bottomTip = "按 ↑↓ 键移动选择，Enter 键确认，ESC 键直接返回";
        int bx = textWidth(bottomTip, 15);
        drawText((W_WIDTH - bx) / 2, W_HEIGHT - 50, bottomTip, 15, RGB(140, 140, 140));

        // 捕获用户键盘交互
        int key = waitForKey();
        if (key >= '1' && key <= '4') {
            selected = key - '1';
        } else if (key == '0') {
            selected = 4;
        }

        if (key == VK_ESCAPE) {
            break;
        } else if (key == VK_UP) {
            selected = (selected - 1 + MENU_COUNT) % MENU_COUNT;
        } else if (key == VK_DOWN) {
            selected = (selected + 1) % MENU_COUNT;
        } else if (key == VK_RETURN || (key >= '0' && key <= '4')) {
            if (selected == 0) {
                // 查看当前指令序列
                addressStream = MemoryManager::generateAddressStream(20);
                while (true) {
                    clearWhite();
                    drawText(40, 40, "当前生成的 20 条指令随机地址流及对应页号:", 22, BLACK);
                    
                    int sy = 90;
                    for (int k = 0; k < 20; ++k) {
                        if (k >= 20) break;
                        std::stringstream ss;
                        ss << "指令 [" << std::setw(2) << std::setfill('0') << (k + 1) << "] -> 逻辑地址: " 
                           << std::setw(3) << std::setfill(' ') << addressStream[k] 
                           << "  |  对应虚页号: " << (addressStream[k] / 10);
                        drawText(60, sy, ss.str(), 15, RGB(50, 50, 50));
                        sy += 25;
                    }
                    
                    std::string escTip = "按任意键返回上一级菜单";
                    drawText(40, W_HEIGHT - 60, escTip, 16, RGB(120, 120, 120));
                    waitForKey();
                    break;
                }
            } else if (selected == 1) {
                // FIFO
                MemoryResult res = manager.runFIFO(addressStream);
                startMemorySimulation(res);
            } else if (selected == 2) {
                // LRU
                MemoryResult res = manager.runLRU(addressStream);
                startMemorySimulation(res);
            } else if (selected == 3) {
                // 修改参数：在内存物理块 3 - 7 之间切换
                frameCount = (frameCount - 3 + 1) % 5 + 3; // 3 -> 4 -> 5 -> 6 -> 7 -> 3
                manager = MemoryManager(frameCount, 1);
                
                // 增加弹窗显式反馈
                clearWhite();
                int dialogW = 400;
                int dialogH = 200;
                int dx = (W_WIDTH - dialogW) / 2;
                int dy = (W_HEIGHT - dialogH) / 2;
                
                setfillcolor(RGB(248, 249, 250));
                setlinecolor(RGB(180, 180, 180));
                fillrectangle(dx, dy, dx + dialogW, dy + dialogH);
                rectangle(dx, dy, dx + dialogW, dy + dialogH);
                
                drawText(dx + 40, dy + 50, "系统参数已成功修改！", 20, RGB(70, 150, 70));
                std::stringstream ss;
                ss << "当前物理块数已调整为: " << frameCount << " 块";
                drawText(dx + 40, dy + 90, ss.str(), 16, BLACK);
                drawText(dx + 40, dy + 130, "按任意键返回主控台...", 14, RGB(140, 140, 140));
                
                waitForKey();
            } else if (selected == 4) {
                break;
            }
        }
    }
}
