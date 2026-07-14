#include <string>
#include <windows.h>
#include <graphics.h>

// ===== 窗口常量（全局统一） =====
constexpr int W_WIDTH = 1000;
constexpr int W_HEIGHT = 800;

// ===== 工具函数 =====

// UTF-8 → ANSI(GBK)
inline std::string utf8ToGbk(const std::string& utf8_str) {
    if (utf8_str.empty()) return "";
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, NULL, 0);
    if (wlen <= 0) return utf8_str;
    std::wstring wstr(wlen - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, &wstr[0], wlen);
    int glen = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (glen <= 0) return utf8_str;
    std::string result(glen - 1, '\0');
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &result[0], glen, NULL, NULL);
    return result;
}

// 等待 EasyX 按键
inline int waitForKey() {
    ExMessage msg;
    while (true) {
        getmessage(&msg, EM_KEY);
        if (msg.message == WM_KEYDOWN) return msg.vkcode;
    }
}

// 在指定位置画文字（自动使用宋体，并设置透明背景避免遮挡）
inline void drawText(int x, int y, const std::string& text, int fontSize, COLORREF color) {
    setbkmode(TRANSPARENT);
    settextstyle(fontSize, 0, utf8ToGbk("宋体").c_str());
    settextcolor(color);
    outtextxy(x, y, utf8ToGbk(text).c_str());
}

// 测量文字宽度
inline int textWidth(const std::string& text, int fontSize) {
    settextstyle(fontSize, 0, utf8ToGbk("宋体").c_str());
    return textwidth(utf8ToGbk(text).c_str());
}

// 清屏为白色
inline void clearWhite() {
    setbkcolor(WHITE);
    cleardevice();
}
