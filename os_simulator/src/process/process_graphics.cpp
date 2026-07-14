#include <graphics.h>
#include <conio.h>
#include "../../include/process_manager.h"
#include "../../include/graphics_utils.h"
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>

// ============================================================
//  演示用默认进程组
// ============================================================
static std::vector<PCB> getDefaultProcesses() {
    std::vector<PCB> procs;
    PCB p0, p1, p2, p3, p4;

    p0.name = "P0"; p0.priority = 5; p0.arrivalTime = 0; p0.burstTime = 4; p0.remainingTime = 4; p0.cpuTime = 0; p0.status = 'W';
    p1.name = "P1"; p1.priority = 3; p1.arrivalTime = 1; p1.burstTime = 3; p1.remainingTime = 3; p1.cpuTime = 0; p1.status = 'W';
    p2.name = "P2"; p2.priority = 4; p2.arrivalTime = 2; p2.burstTime = 5; p2.remainingTime = 5; p2.cpuTime = 0; p2.status = 'W';
    p3.name = "P3"; p3.priority = 2; p3.arrivalTime = 3; p3.burstTime = 2; p3.remainingTime = 2; p3.cpuTime = 0; p3.status = 'W';
    p4.name = "P4"; p4.priority = 1; p4.arrivalTime = 4; p4.burstTime = 6; p4.remainingTime = 6; p4.cpuTime = 0; p4.status = 'W';

    procs.push_back(p0); procs.push_back(p1); procs.push_back(p2);
    procs.push_back(p3); procs.push_back(p4);
    return procs;
}

// ============================================================
//  甘特图绘制
// ============================================================
static void drawGanttChart(const std::vector<std::string>& timeline, int startX, int startY) {
    drawText(startX, startY, "甘特图 (CPU 时间轴):", 16, BLACK);

    // 合并相邻同名时间片
    std::vector<std::pair<std::string, int>> segs;
    for (const auto& t : timeline) {
        if (!segs.empty() && segs.back().first == t) segs.back().second++;
        else segs.push_back({t, 1});
    }

    int cellW = 28;
    int cellH = 28;
    int x = startX;
    int y = startY + 24;
    int currentTime = 0;

    // 用不同的色相区分进程
    COLORREF colors[] = {
        RGB(100,160,220), RGB(220,130,100), RGB(100,200,130),
        RGB(200,160,80),  RGB(160,100,200), RGB(80,180,180)
    };

    for (const auto& seg : segs) {
        int segW = seg.second * cellW;
        // 根据进程编号选色
        int idx = 0;
        if (seg.first.size() >= 2 && seg.first[1] >= '0' && seg.first[1] <= '9')
            idx = (seg.first[1] - '0') % 6;

        setfillcolor(colors[idx]);
        setlinecolor(RGB(60, 60, 60));
        setlinestyle(PS_SOLID, 1);
        fillrectangle(x, y, x + segW, y + cellH);

        std::string label = seg.first;
        int lw = textWidth(label, 13);
        if (segW > lw + 4)
            drawText(x + (segW - lw) / 2, y + 7, label, 13, WHITE);

        // 时间刻度
        drawText(x, y + cellH + 4, std::to_string(currentTime), 11, RGB(80, 80, 80));
        currentTime += seg.second;
        x += segW;
    }
    drawText(x, y + cellH + 4, std::to_string(currentTime), 11, RGB(80, 80, 80));
}

// ============================================================
//  调度结果页（静态展示：统计表 + 甘特图）
// ============================================================
static void drawSchedulingResult(const SchedulingResult& result) {
    clearWhite();

    // 标题
    std::string title = "进程调度模拟 - " + result.algoName;
    int tx = textWidth(title, 22);
    drawText((W_WIDTH - tx) / 2, 18, title, 22, BLACK);

    // 统计数据面板
    setfillcolor(RGB(245, 248, 255));
    setlinecolor(RGB(180, 200, 230));
    fillrectangle(40, 52, W_WIDTH - 40, 88);
    rectangle(40, 52, W_WIDTH - 40, 88);

    std::stringstream ss;
    ss << "平均周转时间: " << std::fixed << std::setprecision(2) << result.avgTurnAroundTime
       << "   |   平均带权周转时间: " << std::fixed << std::setprecision(2)
       << result.avgWeightedTurnAroundTime;
    int sw = textWidth(ss.str(), 16);
    drawText((W_WIDTH - sw) / 2, 63, ss.str(), 16, RGB(30, 60, 120));

    // 表头
    int tX = 40, tY = 105;
    int cw[] = {55, 75, 75, 75, 75, 100};
    const char* hdrs[] = {"进程名","到达时间","服务时间","完成时刻","周转时间","带权周转时间"};

    setfillcolor(RGB(60, 110, 190));
    setlinecolor(RGB(40, 80, 150));
    fillrectangle(tX, tY, tX + 455, tY + 28);

    int hx = tX + 5;
    for (int i = 0; i < 6; ++i) {
        drawText(hx, tY + 7, hdrs[i], 13, WHITE);
        hx += cw[i];
    }

    // 数据行
    for (int i = 0; i < (int)result.finishedProcesses.size(); ++i) {
        const auto& p = result.finishedProcesses[i];
        int rowY = tY + 28 + i * 26;
        setfillcolor(i % 2 == 0 ? WHITE : RGB(245, 248, 255));
        setlinecolor(RGB(210, 215, 225));
        fillrectangle(tX, rowY, tX + 455, rowY + 26);
        rectangle(tX, rowY, tX + 455, rowY + 26);

        std::stringstream wss;
        wss << std::fixed << std::setprecision(2) << p.weightedTurnAroundTime;
        std::string cols[] = {
            p.name,
            std::to_string(p.arrivalTime),
            std::to_string(p.burstTime),
            std::to_string(p.finishTime),
            std::to_string(p.turnAroundTime),
            wss.str()
        };

        int cx = tX + 5;
        for (int j = 0; j < 6; ++j) {
            drawText(cx, rowY + 6, cols[j], 13, BLACK);
            cx += cw[j];
        }
    }

    // 甘特图
    int ganttY = tY + 28 + (int)result.finishedProcesses.size() * 26 + 24;
    drawGanttChart(result.executionTimeline, tX, ganttY);

    std::string tip = "按 【ESC】 返回上级菜单";
    int tipW = textWidth(tip, 15);
    drawText((W_WIDTH - tipW) / 2, W_HEIGHT - 44, tip, 15, RGB(140, 140, 140));
}

// ============================================================
//  动态优先级调度步进可视化
// ============================================================
static void drawDynamicPriorityStep(const std::vector<std::vector<PCB>>& history, int stepIdx) {
    clearWhite();

    drawText(40, 18, "动态优先级调度 (时间片轮转，每片后优先级 -1)", 22, BLACK);

    std::string stepStr = "时间片: " + std::to_string(stepIdx) + " / " + std::to_string((int)history.size() - 1);
    drawText(40, 52, stepStr, 16, RGB(70, 120, 200));

    const auto& procs = history[stepIdx];

    // 表头
    int tX = 40, tY = 86;
    int cw[] = {60, 70, 80, 80, 80, 90, 90};
    const char* hdrs[] = {"进程名","当前优先级","到达时间","服务时间","已用CPU","剩余时间","状态"};

    setfillcolor(RGB(60, 110, 190));
    setlinecolor(RGB(40, 80, 150));
    fillrectangle(tX, tY, tX + 550, tY + 28);

    int hx = tX + 5;
    for (int i = 0; i < 7; ++i) {
        drawText(hx, tY + 7, hdrs[i], 13, WHITE);
        hx += cw[i];
    }

    for (int i = 0; i < (int)procs.size(); ++i) {
        const auto& p = procs[i];
        int rowY = tY + 28 + i * 30;

        COLORREF rowBg = (p.status == 'R') ? RGB(215, 245, 215) :
                         (p.status == 'F') ? RGB(240, 240, 240) :
                         (i % 2 == 0 ? WHITE : RGB(248, 248, 255));
        setfillcolor(rowBg);
        setlinecolor(RGB(205, 210, 220));
        fillrectangle(tX, rowY, tX + 550, rowY + 30);
        rectangle(tX, rowY, tX + 550, rowY + 30);

        COLORREF tc = (p.status == 'R') ? RGB(0, 130, 0) :
                      (p.status == 'F') ? RGB(150, 150, 150) : BLACK;

        std::string statusLabel;
        if (p.status == 'R') statusLabel = "运行中...";
        else if (p.status == 'F') statusLabel = "已完成 !";
        else statusLabel = "就绪 W";

        std::string cols[] = {
            p.name,
            std::to_string(p.priority),
            std::to_string(p.arrivalTime),
            std::to_string(p.burstTime),
            std::to_string(p.cpuTime),
            std::to_string(p.remainingTime),
            statusLabel
        };

        int cx = tX + 5;
        for (int j = 0; j < 7; ++j) {
            drawText(cx, rowY + 8, cols[j], 13, tc);
            cx += cw[j];
        }
    }

    std::string tip = "按 【←/→】 单步步进/回退  |  【Enter】 自动演示  |  【ESC】 返回";
    int tipW = textWidth(tip, 15);
    drawText((W_WIDTH - tipW) / 2, W_HEIGHT - 44, tip, 15, RGB(140, 140, 140));
}

// ============================================================
//  生产者-消费者步进可视化
// ============================================================
static void drawProdConsStep(const std::vector<ProdConsStep>& history, int stepIdx) {
    clearWhite();

    drawText(40, 18, "生产者-消费者问题  (信号量 PV 操作模拟)", 22, BLACK);

    const auto& step = history[stepIdx];

    // 步骤计数器
    std::string stepStr = "步骤: " + std::to_string(stepIdx + 1) + " / " + std::to_string((int)history.size());
    drawText(40, 50, stepStr, 15, RGB(120, 120, 120));

    // 操作者标识
    COLORREF actorColor = (step.actor == "生产者") ? RGB(60, 120, 200) :
                          (step.actor == "消费者") ? RGB(200, 120, 60) : RGB(120, 120, 120);
    std::string actorLabel = step.actor + " → " + step.operation;
    drawText(40, 74, actorLabel, 18, actorColor);

    // 事件描述横幅
    bool isBlock = (step.detail.find("阻塞") != std::string::npos);
    COLORREF bannerBg = isBlock ? RGB(255, 232, 232) : RGB(232, 255, 232);
    COLORREF bannerBd = isBlock ? RGB(200, 80, 80)  : RGB(80, 180, 80);
    COLORREF bannerTc = isBlock ? RGB(170, 30, 30)  : RGB(30, 140, 30);

    setfillcolor(bannerBg); setlinecolor(bannerBd);
    fillrectangle(40, 100, W_WIDTH - 40, 132);
    rectangle(40, 100, W_WIDTH - 40, 132);
    drawText(60, 108, step.detail, 15, bannerTc);

    // 信号量状态面板
    int sY = 150;
    int boxW = 160, boxH = 42, gap = 20;
    int startX = (W_WIDTH - (boxW * 3 + gap * 2)) / 2;

    struct SemItem { const char* name; int val; COLORREF color; };
    SemItem sems[] = {
        {"full",  step.semFull,  RGB(60, 140, 60)},
        {"empty", step.semEmpty, RGB(140, 100, 60)},
        {"mutex", step.semMutex, RGB(180, 60, 60)},
    };

    for (int i = 0; i < 3; ++i) {
        int bx = startX + i * (boxW + gap);
        setfillcolor(RGB(248, 250, 255));
        setlinecolor(RGB(180, 200, 220));
        fillrectangle(bx, sY, bx + boxW, sY + boxH);
        rectangle(bx, sY, bx + boxW, sY + boxH);

        drawText(bx + 10, sY + 4, sems[i].name, 16, RGB(80, 80, 80));
        settextstyle(22, 0, utf8ToGbk("Arial").c_str());
        settextcolor(sems[i].color);
        std::string valStr = std::to_string(sems[i].val);
        outtextxy(bx + boxW - 10 - textWidth(valStr, 22), sY + 9, utf8ToGbk(valStr).c_str());
    }

    // 缓冲池图形
    int poolY = 210;
    int poolX = 40;
    int cW = 56, cH = 42;
    int maxPerRow = (W_WIDTH - 80) / (cW + 6);
    int cap = step.pool.size();

    for (int i = 0; i < cap; ++i) {
        int row = i / maxPerRow;
        int col = i % maxPerRow;
        int cx = poolX + col * (cW + 6);
        int cy = poolY + row * (cH + 6);

        bool hasProd = (step.pool[i] != "[Empty]");
        setfillcolor(hasProd ? RGB(170, 225, 170) : RGB(248, 248, 248));
        setlinecolor(hasProd ? RGB(70, 155, 70) : RGB(200, 200, 200));
        setlinestyle(PS_SOLID, 2);
        fillrectangle(cx, cy, cx + cW, cy + cH);

        std::string label = hasProd ? "P" : "";
        int lw = textWidth(label, 16);
        drawText(cx + (cW - lw) / 2, cy + 10, label, 16, hasProd ? RGB(30, 110, 30) : RGB(210, 210, 210));

        std::string idx = std::to_string(i);
        drawText(cx + (cW - textWidth(idx, 11)) / 2, cy + cH - 14, idx, 11, RGB(160, 160, 160));
    }

    // 图例 + 提示
    int ly = std::max(poolY + ((cap + maxPerRow - 1) / maxPerRow) * (cH + 6) + 10, W_HEIGHT - 100);
    setlinecolor(RGB(200, 200, 200)); setlinestyle(PS_SOLID, 1);
    line(40, ly, W_WIDTH - 40, ly);

    setfillcolor(RGB(170, 225, 170)); setlinecolor(RGB(70, 155, 70));
    fillrectangle(40, ly + 12, 60, ly + 28);
    drawText(68, ly + 12, "= 已有商品", 13, BLACK);

    setfillcolor(RGB(248, 248, 248)); setlinecolor(RGB(200, 200, 200));
    fillrectangle(180, ly + 12, 200, ly + 28);
    drawText(208, ly + 12, "= 空闲槽位", 13, BLACK);

    std::string tip = "按 【←/→】 或 【A/D】 步进/回退  |  【Enter】 自动演示  |  【ESC】 返回";
    int tipW = textWidth(tip, 15);
    drawText((W_WIDTH - tipW) / 2, ly + 45, tip, 15, RGB(140, 140, 140));
}

// ============================================================
//  通用交互演示控制器
// ============================================================
static void runSchedulingView(const SchedulingResult& result) {
    drawSchedulingResult(result);
    while (waitForKey() != VK_ESCAPE);
}

template<typename Fn>
static void runStepView(int total, Fn drawFn) {
    int step = 0;
    bool autoPlay = false;

    while (true) {
        drawFn(step);

        if (autoPlay) {
            Sleep(650);
            if (step < total - 1) step++;
            else autoPlay = false;
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
        if (key == VK_ESCAPE) break;
        else if (key == VK_RIGHT || key == 'D' || key == 'd') { if (step < total - 1) step++; }
        else if (key == VK_LEFT  || key == 'A' || key == 'a') { if (step > 0) step--; }
        else if (key == VK_RETURN) { autoPlay = true; if (step == total - 1) step = 0; }
    }
}

// ============================================================
//  进程管理二级主菜单入口
// ============================================================
void processManagerSubMenu() {
    std::vector<PCB> processes = getDefaultProcesses();
    int selected = 0;
    const int MENU_COUNT = 6;

    while (true) {
        clearWhite();

        int tx = textWidth("进程管理模拟子系统", 28);
        drawText((W_WIDTH - tx) / 2, 40, "进程管理模拟子系统", 28, BLACK);

        std::string paramStr = "演示进程组: P0~P4，各进程具有不同的优先级 / 到达时间 / 服务时间";
        tx = textWidth(paramStr, 15);
        drawText((W_WIDTH - tx) / 2, 90, paramStr, 15, RGB(100, 100, 100));

        setlinecolor(RGB(200, 200, 200));
        line(150, 120, W_WIDTH - 150, 120);

        struct MI { std::string label; std::string desc; };
        MI items[MENU_COUNT] = {
            {"1. 生产者-消费者问题模拟",         "基于 PV 操作信号量机制，可视化缓冲池状态逐步变化"},
            {"2. 先来先服务 (FCFS)",             "按进程到达顺序依次占用 CPU，输出甘特图与周转时间"},
            {"3. 短作业优先 (SJF)",              "优先调度服务时间最短的就绪进程，输出甘特图与周转时间"},
            {"4. 最高响应比优先 (HRRN)",         "动态计算响应比 = (等待时间+服务时间)/服务时间，取最大"},
            {"5. 动态优先级调度 (时间片轮转)",   "每时间片后优先级 -1 重新排队，可逐步回放 PCB 变化"},
            {"0. 返回主菜单",                    ""}
        };

        int startY = 160;
        for (int i = 0; i < MENU_COUNT; ++i) {
            bool isSel = (i == selected);
            if (isSel) {
                setfillcolor(RGB(230, 240, 255));
                setlinecolor(RGB(230, 240, 255));
                fillrectangle(100, startY - 5, W_WIDTH - 100, startY + 38);
            }

            drawText(130, startY, items[i].label, 20, BLACK);
            if (!items[i].desc.empty())
                drawText(130, startY + 22, items[i].desc, 13, RGB(130, 130, 130));

            startY += 60;
        }

        std::string bottomTip = "按 ↑↓ 键移动选择，Enter / 数字键 确认，ESC 直接返回";
        int bx = textWidth(bottomTip, 15);
        drawText((W_WIDTH - bx) / 2, W_HEIGHT - 44, bottomTip, 15, RGB(140, 140, 140));

        int key = waitForKey();
        if (key >= '1' && key <= '5') selected = key - '1';
        else if (key == '0') selected = 5;

        if (key == VK_ESCAPE) break;
        else if (key == VK_UP)   selected = (selected - 1 + MENU_COUNT) % MENU_COUNT;
        else if (key == VK_DOWN) selected = (selected + 1) % MENU_COUNT;
        else if (key == VK_RETURN || (key >= '0' && key <= '5')) {
            if (selected == 0) {
                auto hist = ProcessManager::startProdConsThreads(30, 1, 3);
                int total = hist.size();
                runStepView(total, [&](int s) { drawProdConsStep(hist, s); });
            } else if (selected == 1) {
                runSchedulingView(ProcessManager::runFCFS(processes));
            } else if (selected == 2) {
                runSchedulingView(ProcessManager::runSJF(processes));
            } else if (selected == 3) {
                runSchedulingView(ProcessManager::runHRRN(processes));
            } else if (selected == 4) {
                auto hist = ProcessManager::runDynamicPriority(processes, 1);
                int total = hist.size();
                runStepView(total, [&](int s) { drawDynamicPriorityStep(hist, s); });
            } else if (selected == 5) {
                break;
            }
        }
    }
}
