#include <graphics.h>
#include "disk_scheduler.h"
#include "graphics_utils.h"
#include <sstream>
#include <iomanip>

// ===== 绘图配置 =====
struct ChartConfig {
    static constexpr int ML = 80, MR = 50, MT = 60, MB = 80;
    static constexpr COLORREF BG  = RGB(245,245,245);
    static constexpr COLORREF GRID= RGB(200,200,200);
    static constexpr COLORREF AXIS= RGB(50,50,50);
    static constexpr COLORREF HEAD= RGB(220,50,50);
    static constexpr COLORREF LINE= RGB(70,130,180);
    static constexpr COLORREF PT  = RGB(255,165,0);
    static constexpr COLORREF TXT = RGB(30,30,30);
    static constexpr COLORREF INFO= RGB(240,240,240);
};

// ===== 坐标转换 =====
POINT toPixel(int idx, int track, int n, int max_t, int cw, int ch) {
    POINT p;
    p.x = ChartConfig::ML + (n > 1 ? idx * cw / (n - 1) : cw / 2);
    p.y = ChartConfig::MT + ch - track * ch / max_t;
    return p;
}

// ===== 生成随机请求 =====
std::vector<int> genReqs(int count, int max_t) {
    std::vector<int> r;
    for (int i = 0; i < count; ++i) r.push_back(rand() % (max_t + 1));
    return r;
}

// ===== 绘制调度图表 =====
void drawChart(const ScheduleResult& res, const std::vector<int>& reqs,
               int head0, int max_t) {
    cleardevice();
    setbkcolor(ChartConfig::BG);
    cleardevice();

    int cw = W_WIDTH - ChartConfig::ML - ChartConfig::MR;
    int ch = W_HEIGHT - ChartConfig::MT - ChartConfig::MB;

    // 标题
    std::string title = "磁盘调度算法可视化 - " + res.algo_name +
                        " (" + res.algo_short_name + ")";
    int tx = textWidth(title, 24);
    drawText((W_WIDTH - tx) / 2, 12, title, 24, ChartConfig::TXT);

    // 坐标系
    setlinecolor(ChartConfig::AXIS);
    setlinestyle(PS_SOLID, 2);
    line(ChartConfig::ML, ChartConfig::MT, ChartConfig::ML, ChartConfig::MT + ch);
    line(ChartConfig::ML, ChartConfig::MT + ch, ChartConfig::ML + cw, ChartConfig::MT + ch);

    drawText(ChartConfig::ML - 50, ChartConfig::MT - 30, "磁道号", 16, ChartConfig::TXT);
    drawText(W_WIDTH - ChartConfig::MR - 40, ChartConfig::MT + ch + 20, "请求序列", 16, ChartConfig::TXT);

    // 网格
    setlinecolor(ChartConfig::GRID);
    setlinestyle(PS_SOLID, 1);
    int step = max_t > 100 ? 20 : 10;
    for (int i = 0; i <= max_t; i += step) {
        int y = ChartConfig::MT + ch - i * ch / max_t;
        line(ChartConfig::ML - 5, y, ChartConfig::ML, y);
        line(ChartConfig::ML, y, ChartConfig::ML + cw, y);
        drawText(ChartConfig::ML - 35, y - 8, std::to_string(i), 14, ChartConfig::TXT);
    }

    int n = res.sequence.size();
    for (int i = 0; i < n; ++i) {
        int x = ChartConfig::ML + i * cw / (n - 1);
        line(x, ChartConfig::MT + ch, x, ChartConfig::MT + ch + 5);
        if (i % 5 == 0 || i == n - 1 || i == 0)
            drawText(x - 10, ChartConfig::MT + ch + 12, std::to_string(i), 14, ChartConfig::TXT);
    }

    // 轨迹线（动画）
    setlinecolor(ChartConfig::LINE);
    setlinestyle(PS_SOLID, 3);
    for (int i = 1; i < n; ++i) {
        POINT a = toPixel(i - 1, res.sequence[i - 1], n, max_t, cw, ch);
        POINT b = toPixel(i,     res.sequence[i],     n, max_t, cw, ch);
        line(a.x, a.y, b.x, b.y);
        Sleep(120);
    }

    // 访问点
    setfillcolor(ChartConfig::PT);
    setlinecolor(ChartConfig::PT);
    for (int i = 0; i < n; ++i) {
        POINT p = toPixel(i, res.sequence[i], n, max_t, cw, ch);
        fillcircle(p.x, p.y, 6);
        drawText(p.x - 12, p.y - 22, std::to_string(res.sequence[i]), 12, ChartConfig::TXT);
    }

    // 初始磁头
    POINT hp = toPixel(0, head0, n, max_t, cw, ch);
    setfillcolor(ChartConfig::HEAD);
    setlinecolor(ChartConfig::HEAD);
    fillcircle(hp.x, hp.y, 10);
    drawText(hp.x - 18, hp.y + 18, "初始", 14, ChartConfig::TXT);

    // 统计面板
    setfillcolor(ChartConfig::INFO);
    setlinecolor(ChartConfig::AXIS);
    setlinestyle(PS_SOLID, 1);
    int iy = W_HEIGHT - 55;
    rectangle(ChartConfig::ML, iy, ChartConfig::ML + cw, iy + 40);

    std::stringstream ss;
    ss << "总寻道: " << res.total_seek << "  ";
    drawText(ChartConfig::ML + 20, iy + 10, ss.str(), 16, ChartConfig::TXT);

    ss.str("");
    ss << std::fixed << std::setprecision(2) << "平均寻道: " << res.avg_seek << "  ";
    drawText(ChartConfig::ML + 220, iy + 10, ss.str(), 16, ChartConfig::TXT);

    ss.str("");
    ss << "请求数: " << reqs.size() << "  ";
    drawText(ChartConfig::ML + 460, iy + 10, ss.str(), 16, ChartConfig::TXT);

    ss.str("");
    ss << "初始磁头: " << head0;
    drawText(ChartConfig::ML + 640, iy + 10, ss.str(), 16, ChartConfig::TXT);

    drawText(W_WIDTH - 260, W_HEIGHT - 25, "按 ESC 返回上级菜单", 14, ChartConfig::TXT);
}

// ================================================================
//  磁盘调度二级菜单
// ================================================================
void diskSchedulerSubMenu() {
    std::vector<int> reqs = {55, 58, 39, 18, 90, 160, 150, 38, 184};
    int head0 = 100;
    const int MAX_T = 200;
    const int MENU_COUNT = 5;

    struct AlgoItem { char key; std::string label; };
    AlgoItem opts[] = {
        {'1', "1. FCFS  (先来先服务)"},
        {'2', "2. SSTF  (最短寻道时间优先)"},
        {'3', "3. SCAN  (扫描算法/电梯)"},
        {'4', "4. CSCAN (循环扫描算法)"},
        {'0', "0.  返回主菜单"},
    };
    const int OPT_COUNT = sizeof(opts) / sizeof(opts[0]);

    int selected = 0;

    while (true) {
        clearWhite();

        int tw0 = textWidth("磁盘调度算法可视化演示", 28);
        drawText((W_WIDTH - tw0) / 2, 40, "磁盘调度算法可视化演示", 28, BLACK);

        std::string s = "初始磁头位置: " + std::to_string(head0);
        drawText(80, 100, s, 16, BLACK);

        s = "磁道请求序列: ";
        for (int r : reqs) s += std::to_string(r) + " ";
        drawText(80, 130, s, 16, BLACK);

        drawText(80, 180, "请选择磁盘调度算法:", 20, BLACK);

        int my = 220;
        for (int i = 0; i < OPT_COUNT; ++i) {
            if (i == selected) {
                setfillcolor(RGB(230, 240, 255));
                setlinecolor(RGB(230, 240, 255));
                fillrectangle(100, my - 3, 500, my + 25);
            }
            drawText(120, my, opts[i].label, 18, BLACK);
            my += 42;
        }
        drawText(80, 460, "按 ↑↓ 选择，按 Enter 确认，或直接按 1-4 / 0 快捷选择", 15, RGB(140,140,140));

        int key = waitForKey();

        if (key == VK_ESCAPE) break;

        DiskAlgorithm algo;
        bool runAlgo = false;

        if (key >= '0' && key <= '4') {
            if (key == '0') break;
            switch (key) {
                case '1': algo = DiskAlgorithm::FCFS;  break;
                case '2': algo = DiskAlgorithm::SSTF;  break;
                case '3': algo = DiskAlgorithm::SCAN;  break;
                case '4': algo = DiskAlgorithm::CSCAN; break;
            }
            runAlgo = true;
        }

        if (key == VK_UP && selected > 0) selected--;
        if (key == VK_DOWN && selected < OPT_COUNT - 1) selected++;

        if (key == VK_RETURN || key == VK_SPACE) {
            int k = opts[selected].key;
            if (k == '0') break;
            switch (k) {
                case '1': algo = DiskAlgorithm::FCFS;  break;
                case '2': algo = DiskAlgorithm::SSTF;  break;
                case '3': algo = DiskAlgorithm::SCAN;  break;
                case '4': algo = DiskAlgorithm::CSCAN; break;
                default: continue;
            }
            runAlgo = true;
        }

        if (!runAlgo) continue;

        DiskScheduler sched(reqs, head0);
        sched.setMaxTrack(MAX_T);
        auto result = sched.run(algo);
        sched.printResult(result);
        drawChart(result, reqs, head0, MAX_T);

        while (waitForKey() != VK_ESCAPE);
    }
}
