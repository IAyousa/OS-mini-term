#include <graphics.h>
#include "device_manager.h"
#include "graphics_utils.h"
#include <sstream>
#include <string>
#include <vector>

// 当前系统状态（全局，在子菜单生命周期内保持）
static BankerSystem g_sys;

// ============================================================
//  绘制系统状态表格
// ============================================================
void drawSystemTable() {
    int M = g_sys.processCount;
    int N = g_sys.resourceCount;

    // 表头
    int colX[4] = {30, 160, 340, 520};
    int rowY = 90;
    int cellH = 28;

    drawText(colX[0], rowY, "进程", 16, BLACK);
    drawText(colX[1], rowY, "MAX", 16, BLACK);
    drawText(colX[2], rowY, "Allocation", 16, BLACK);
    drawText(colX[3], rowY, "Need", 16, BLACK);
    rowY += cellH;

    // 分隔线
    setlinecolor(RGB(180, 180, 180));
    line(20, rowY - 5, 700, rowY - 5);

    // 数据行
    for (int i = 0; i < M; ++i) {
        std::string label = "P" + std::to_string(i);
        drawText(colX[0], rowY, label, 15, BLACK);

        std::string maxStr, allStr, needStr;
        for (int j = 0; j < N; ++j) {
            if (j > 0) { maxStr += " "; allStr += " "; needStr += " "; }
            maxStr  += std::to_string(g_sys.max[i][j]);
            allStr  += std::to_string(g_sys.allocation[i][j]);
            needStr += std::to_string(g_sys.need[i][j]);
        }
        drawText(colX[1], rowY, maxStr, 15, RGB(50, 50, 50));
        drawText(colX[2], rowY, allStr, 15, RGB(50, 120, 50));
        drawText(colX[3], rowY, needStr, 15, RGB(180, 50, 50));
        rowY += cellH;
    }

    // Available
    rowY += 5;
    std::string availStr = "Available: ";
    for (int j = 0; j < N; ++j) {
        if (j > 0) availStr += "  ";
        availStr += std::to_string(g_sys.available[j]);
    }
    drawText(30, rowY, availStr, 16, RGB(0, 100, 200));
}

// ============================================================
//  输入编辑器：在指定位置获取用户数字输入
//  返回用户输入的数字值，-1 表示取消
// ============================================================
int inputNumber(int x, int y, const std::string& prompt, int maxValue) {
    // 画提示
    drawText(x, y, prompt, 16, BLACK);

    // 画输入框
    int boxX = x + textWidth(prompt, 16) + 10;
    int boxW = 80, boxH = 24;

    std::string input;
    std::string errorMsg;
    int errorTimer = 0;

    ExMessage msg;
    while (true) {
        // 画输入框背景
        setfillcolor(RGB(245, 250, 255));
        setlinecolor(RGB(100, 140, 200));
        fillrectangle(boxX, y - 2, boxX + boxW, y + boxH);
        rectangle(boxX, y - 2, boxX + boxW, y + boxH);

        // 显示已输入内容
        drawText(boxX + 4, y, input, 16, BLACK);

        // 光标闪烁
        if ((errorTimer / 10) % 2 == 0) {
            int cx = boxX + 4 + textWidth(input, 16);
            setlinecolor(BLACK);
            line(cx, y, cx, y + boxH - 4);
        }

        // ---- 错误提示区域 ----
        setfillcolor(WHITE);
        setlinecolor(WHITE);
        fillrectangle(boxX, y + boxH + 2, boxX + boxW + 150, y + boxH + 22);

        if (!errorMsg.empty()) {
            settextcolor(RGB(220, 50, 50));
            settextstyle(13, 0, utf8ToGbk("宋体").c_str());
            outtextxy(boxX, y + boxH + 4, utf8ToGbk(errorMsg).c_str());
            errorTimer++;
            if (errorTimer > 60) {  // ~3 秒
                errorMsg.clear();
                errorTimer = 0;
            }
        }

        // ---- 非阻塞检测按键 ----
        if (peekmessage(&msg, EM_KEY)) {
            if (msg.message == WM_KEYDOWN) {
                int key = msg.vkcode;

                if (key == VK_ESCAPE) return -1;

                if (key == VK_RETURN || key == VK_SPACE) {
                    if (!input.empty()) {
                        int val = std::stoi(input);
                        if (val >= 0 && val <= maxValue) {
                            // 返回前擦除光标
                            setfillcolor(RGB(245, 250, 255));
                            setlinecolor(RGB(100, 140, 200));
                            fillrectangle(boxX, y - 2, boxX + boxW, y + boxH);
                            drawText(boxX + 4, y, input, 16, BLACK);
                            rectangle(boxX, y - 2, boxX + boxW, y + boxH);
                            return val;
                        }
                        errorMsg = "超出范围 (0~" + std::to_string(maxValue) + ")";
                        errorTimer = 0;
                        input.clear();
                    }
                    continue;
                }

                if (key >= '0' && key <= '9') {
                    if (input.length() < 4) input += (char)key;
                }

                if (key == VK_BACK && !input.empty()) input.pop_back();
            }
        }

        Sleep(50);  // 控制轮询频率 (~20次/秒)
    }
}

// ============================================================
//  输入 N 个资源值（用于 Request 向量）
// ============================================================
bool inputRequestVector(int x, int y, int processIdx, std::vector<int>& request) {
    int N = g_sys.resourceCount;
    int spacing = 130;
    request.assign(N, 0);

    // 在输入区上方显示"当前选中进程"及相关提示
    std::string selMsg = "> 当前选中: 进程 P" + std::to_string(processIdx) +
                         "  (资源上限: Need=";
    for (int j = 0; j < N; ++j) {
        if (j > 0) selMsg += ",";
        selMsg += std::to_string(g_sys.need[processIdx][j]);
    }
    selMsg += "  Avail=";
    for (int j = 0; j < N; ++j) {
        if (j > 0) selMsg += ",";
        selMsg += std::to_string(g_sys.available[j]);
    }
    selMsg += ")";
    drawText(x, y - 30, selMsg, 15, RGB(0, 100, 200));

    for (int j = 0; j < N; ++j) {
        std::string prompt = "R" + std::to_string(j) + ":";
        int val = inputNumber(x + j * spacing, y, prompt,
                              g_sys.need[processIdx][j]);
        if (val < 0) return false;  // ESC 取消
        request[j] = val;
    }
    return true;
}

// ============================================================
//  系统设置界面：让用户自定义 M(进程数) 和 N(资源种类数)
// ============================================================
bool setupSystem() {
    int M = 5, N = 3;  // 默认值
    int selected = 0;  // 0=M, 1=N

    while (true) {
        clearWhite();

        // 标题
        int tw0 = textWidth("设备管理 — 系统设置", 26);
        drawText((W_WIDTH - tw0) / 2, 40, "设备管理 — 系统设置", 26, BLACK);

        drawText(80, 120, "请设置系统参数，设置完成后按 Enter 进入演示：", 18, BLACK);

        // ---- M 输入 ----
        int labelM_X = 80, boxY1 = 175, boxW = 80, boxH = 28;
        drawText(labelM_X, boxY1 + 2, "进程数量 M :", 18, BLACK);
        int mBoxX = labelM_X + textWidth("进程数量 M :", 18) + 10;

        // 选中高亮
        if (selected == 0) {
            setfillcolor(RGB(220, 235, 255));
            setlinecolor(RGB(50, 100, 200));
        } else {
            setfillcolor(RGB(245, 250, 255));
            setlinecolor(RGB(150, 180, 220));
        }
        fillrectangle(mBoxX, boxY1, mBoxX + boxW, boxY1 + boxH);
        rectangle(mBoxX, boxY1, mBoxX + boxW, boxY1 + boxH);
        drawText(mBoxX + 6, boxY1 + 3, std::to_string(M), 18, BLACK);

        // ---- N 输入 ----
        int labelN_X = 80, boxY2 = 225;
        drawText(labelN_X, boxY2 + 2, "资源种类 N :", 18, BLACK);
        int nBoxX = labelN_X + textWidth("资源种类 N :", 18) + 10;

        if (selected == 1) {
            setfillcolor(RGB(220, 235, 255));
            setlinecolor(RGB(50, 100, 200));
        } else {
            setfillcolor(RGB(245, 250, 255));
            setlinecolor(RGB(150, 180, 220));
        }
        fillrectangle(nBoxX, boxY2, nBoxX + boxW, boxY2 + boxH);
        rectangle(nBoxX, boxY2, nBoxX + boxW, boxY2 + boxH);
        drawText(nBoxX + 6, boxY2 + 3, std::to_string(N), 18, BLACK);

        drawText(80, 285, "按 ↑↓ 切换选项，按 ←→ 调整数值，按 Enter 确认，按 ESC 返回", 15, RGB(140, 140, 140));

        int key = waitForKey();
        if (key == VK_ESCAPE) return false;

        if (key == VK_RETURN || key == VK_SPACE) {
            if (M >= 1 && M <= 10 && N >= 1 && N <= 5) {
                initSystem(g_sys, M, N);
                return true;
            }
        }

        // 切换选中项
        if (key == VK_UP || key == VK_DOWN)
            selected = 1 - selected;

        // 调整数值
        if (selected == 0) {  // M
            if (key == VK_RIGHT && M < 10) M++;
            if (key == VK_LEFT  && M > 1)  M--;
            if (key >= '1' && key <= '9')  M = key - '0';
            if (key == '0') M = 10;
        } else {  // N
            if (key == VK_RIGHT && N < 5) N++;
            if (key == VK_LEFT  && N > 1) N--;
            if (key >= '1' && key <= '5')  N = key - '0';
        }
    }
}

// ============================================================
//  主交互循环
// ============================================================
void deviceManagerSubMenu() {
    // 先进入设置界面
    if (!setupSystem()) return;

    g_sys.lastMessage = "就绪 — 请选择进程并输入请求";

    while (true) {
        clearWhite();

        // ---- 标题 ----
        settextstyle(26, 0, utf8ToGbk("宋体").c_str());
        settextcolor(BLACK);
        int tw0 = textWidth(utf8ToGbk("设备管理 — 银行家算法"), 26);
        drawText((W_WIDTH - tw0) / 2, 20, "设备管理 — 银行家算法", 26, BLACK);

        // ---- 状态表格 ----
        drawSystemTable();

        // ---- 上次结果信息（动态定位，避免和系统表格重叠） ----
        int msgY = 90 + 28 + g_sys.processCount * 28 + 5 + 16 + 15;
        if (msgY < 320) msgY = 320;
        drawText(30, msgY, g_sys.lastMessage, 15,
                 g_sys.lastRequestGranted ? RGB(0, 130, 0) : RGB(200, 50, 50));

        // ---- 操作提示 ----
        int hintY = msgY + 30;
        drawText(30, hintY, "操作步骤：", 16, RGB(80, 80, 80));
        drawText(30, hintY + 25, "① 按数字键 0~" + std::to_string(g_sys.processCount - 1) +
                          " 选择请求资源的进程", 15, RGB(100, 100, 100));
        drawText(30, hintY + 50, "② 依次输入各资源请求值（回车确认）", 15, RGB(100, 100, 100));
        drawText(30, hintY + 75, "③ 系统自动检查并展示分配结果", 15, RGB(100, 100, 100));

        drawText(30, hintY + 110, "按 R 重置系统   |   按 0~" +
                 std::to_string(g_sys.processCount - 1) + " 选择进程   |   按 ESC 返回",
                 14, RGB(140, 140, 140));
        // ---- 等待用户按键 ----
        int key = waitForKey();

        if (key == VK_ESCAPE) break;

        if (key == 'R' || key == 'r') {
            // 重置系统
            initSystem(g_sys);
            g_sys.lastMessage = "系统已重置";
            continue;
        }

        // 选择进程
        int processIdx = key - '0';
        if (processIdx < 0 || processIdx >= g_sys.processCount) continue;

        // 显示选中提示（先清除旧文字背景，避免透明模式残留）
        setfillcolor(WHITE);
        setlinecolor(WHITE);
        fillrectangle(20, msgY - 2, 700, msgY + 20);
        g_sys.lastMessage = "已选择进程 P" + std::to_string(processIdx) +
                            "，请在下方输入各资源请求值 (上限为 Need 和 Available 的较小值)";
        drawText(30, msgY, g_sys.lastMessage, 15, RGB(0, 100, 200));

        // 清除底部提示区域，为输入区腾出空间
        setfillcolor(WHITE);
        setlinecolor(WHITE);
        fillrectangle(20, msgY + 25, W_WIDTH - 20, W_HEIGHT - 10);
        int inputY = msgY + 40;

        // ---- 输入请求 ----
        std::vector<int> request;
        bool completed = inputRequestVector(30, inputY + 30, processIdx, request);
        if (!completed) {
            g_sys.lastMessage = "已取消请求";
            continue;
        }

        // 检查是否全零请求（无意义的请求）
        bool allZero = true;
        for (int v : request) if (v != 0) { allZero = false; break; }
        if (allZero) {
            g_sys.lastMessage = "请求向量全为零，跳过";
            continue;
        }

        // ---- 执行银行家算法 ----
        tryAllocate(g_sys, processIdx, request);
    }
}
