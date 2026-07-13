#include <cstdlib>
#include <ctime>
#include <string>
#include <graphics.h>
#include <conio.h>
#include "graphics_utils.h"

// ================================================================
//  子菜单声明
// ================================================================
void deviceManagerSubMenu();
void diskSchedulerSubMenu();
void memoryManagerSubMenu();
void processManagerSubMenu();

// ================================================================
//  开发中提示
// ================================================================
void showComingSoon(const std::string& moduleName) {
    clearWhite();

    int txt_w = textWidth(moduleName, 28);
    drawText((W_WIDTH - txt_w) / 2, 200, moduleName, 28, BLACK);
    txt_w = textWidth("该模块正在开发中，敬请期待...", 22);
    drawText((W_WIDTH - txt_w) / 2, 260, "该模块正在开发中，敬请期待...", 22, RGB(180,180,180));
    txt_w = textWidth("按 ESC 返回主菜单", 16);
    drawText((W_WIDTH - txt_w) / 2, 330, "按 ESC 返回主菜单", 16, RGB(140,140,140));

    while (waitForKey() != VK_ESCAPE);
}

// ================================================================
//  菜单项结构
// ================================================================
struct MenuItem {
    char key;
    const char* label;
    const char* status;
    COLORREF statusColor;
};

static const MenuItem MENU_ITEMS[] = {
    {'1', "进程管理",              "已完成",    RGB(70,150,70)},
    {'2', "存储管理",              "已完成",    RGB(70,150,70)},
    {'3', "设备管理",              "已完成",    RGB(70,150,70)},
    {'4', "文件管理",              "已完成",    RGB(70,150,70)},
    {'0', "退出",                  "",         RGB(200,60,60)},
};
static const int MENU_COUNT = sizeof(MENU_ITEMS) / sizeof(MENU_ITEMS[0]);

// ================================================================
//  一级菜单：主界面
// ================================================================
void showMainMenu(int selectedIndex) {
    clearWhite();

    // 标题
    int txt_w = textWidth("操作系统模拟器", 36);
    drawText((W_WIDTH - txt_w) / 2, 60, "操作系统模拟器", 36, BLACK);
    txt_w = textWidth("—— 课程设计 ——", 18);
    drawText((W_WIDTH - txt_w) / 2, 110, "—— 课程设计 ——", 18, RGB(120,120,120));

    // 分割线
    setlinecolor(RGB(200, 200, 200));
    line(150, 150, W_WIDTH - 150, 150);

    // 菜单项
    int y = 190;
    for (int i = 0; i < MENU_COUNT; ++i) {
        const auto& item = MENU_ITEMS[i];
        bool isSelected = (i == selectedIndex);
        bool isExit = (item.key == '0');

        // 选中高亮背景
        if (isSelected) {
            setfillcolor(RGB(230, 240, 255));
            setlinecolor(RGB(230, 240, 255));
            fillrectangle(130, y - 3, 600, y + 30);
        }

        // 编号 + 模块名（纯文字）
        std::string line = std::string(1, item.key) + ". " + item.label;
        drawText(160, y, line, 22, BLACK);

        // 状态标签（退出项不显示）
        if (!isExit) {
            drawText(480, y + 3, item.status, 16, item.statusColor);
        }

        y += 48;
    }

    // 底部提示
    txt_w = textWidth("按 ↑↓ 移动选择，按 Enter 确认，按数字键快捷选择", 15);
    drawText((W_WIDTH - txt_w) / 2, W_HEIGHT - 50,
             "按 ↑↓ 移动选择，按 Enter 确认，按数字键快捷选择", 15, RGB(140,140,140));
}

// ================================================================
//  运行菜单导航（返回选中项对应的 key 字符，或特殊值）
// ================================================================
char navigateMainMenu() {
    int selected = 0;  // 默认选中第一个

    while (true) {
        showMainMenu(selected);
        int key = waitForKey();

        // 数字快捷键
        if (key >= '0' && key <= '4') return (char)key;
        if (key == VK_ESCAPE) return '0';

        // 方向键
        if (key == VK_UP && selected > 0) selected--;
        if (key == VK_DOWN && selected < MENU_COUNT - 1) selected++;

        // Enter 确认
        if (key == VK_RETURN || key == VK_SPACE)
            return MENU_ITEMS[selected].key;
    }
}

// ================================================================
//  主入口
// ================================================================
void runGraphicsApp() {
    initgraph(W_WIDTH, W_HEIGHT);

    while (true) {
        char choice = navigateMainMenu();

        if (choice == '0') break;

        switch (choice) {
            case '1': processManagerSubMenu();  break;
            case '2': memoryManagerSubMenu();   break;
            case '3': deviceManagerSubMenu();   break;
            case '4': diskSchedulerSubMenu();   break;
        }
    }

    closegraph();
}

int main() {
    system("chcp 65001 > nul");
    srand((unsigned)time(NULL));
    runGraphicsApp();
    return 0;
}
