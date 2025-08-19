// Copyright(C) 2025 YAMADA RYO
// 此工具用于将 ExtraOptions 获取的坐标十六进制方向值转换为角度以便写入 Attrib 数据库
// 该代码根据 MIT 许可证发布
// 著作权所有
#include <windows.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <shellscalingapi.h>

#pragma comment(lib, "Shcore.lib")

#define IDC_INPUT 101
#define IDC_OUTPUT 102
#define IDC_STATIC_INPUT 103
#define IDC_STATIC_OUTPUT 104

HFONT hFont;

std::wstring ConvertHexToAngle(const std::wstring& hexStr) {
    try {
        std::wstring hex = hexStr;
        if (hex.substr(0, 2) == L"0x" || hex.substr(0, 2) == L"0X")
            hex = hex.substr(2);

        unsigned int value = std::stoul(hex, nullptr, 16);
        double angle = (static_cast<double>(value) / 65536.0) * 360.0;

        std::wstringstream ss;
        ss << std::fixed << std::setprecision(4) << angle;
        return ss.str();
    }
    catch (...) {
        return L"无效的输入";
    }
}

int DpiScale(int value, UINT dpi) {
    return MulDiv(value, dpi, 96);
}

void ResizeControls(HWND hwnd, UINT dpi) {

    HWND hStaticInput = GetDlgItem(hwnd, IDC_STATIC_INPUT);
    HWND hStaticOutput = GetDlgItem(hwnd, IDC_STATIC_OUTPUT);
    HWND hInput = GetDlgItem(hwnd, IDC_INPUT);
    HWND hOutput = GetDlgItem(hwnd, IDC_OUTPUT);

    const int margin_x = 20;
    const int margin_y = 20;
    const int control_width = 200;
    const int control_height = 25;
    const int label_height = 20;
    const int spacing_y = 15;

    MoveWindow(hStaticInput, DpiScale(margin_x, dpi), DpiScale(margin_y, dpi), DpiScale(control_width, dpi), DpiScale(label_height, dpi), TRUE);
    MoveWindow(hInput, DpiScale(margin_x, dpi), DpiScale(margin_y + label_height, dpi), DpiScale(control_width, dpi), DpiScale(control_height, dpi), TRUE);

    MoveWindow(hStaticOutput, DpiScale(margin_x, dpi), DpiScale(margin_y + label_height + control_height + spacing_y, dpi), DpiScale(control_width, dpi), DpiScale(label_height, dpi), TRUE);
    MoveWindow(hOutput, DpiScale(margin_x, dpi), DpiScale(margin_y + label_height + control_height + spacing_y + label_height, dpi), DpiScale(control_width, dpi), DpiScale(control_height, dpi), TRUE);
}


void RefreshFonts(HWND hwnd, UINT dpi) {
    if (hFont) {
        DeleteObject(hFont);
    }
    int scaledFontHeight = -DpiScale(14, dpi);
    hFont = CreateFontW(
        scaledFontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"黑体"
    );

    EnumChildWindows(hwnd, [](HWND hChild, LPARAM lParam) {
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);
        return TRUE;
        }, 0);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static UINT dpi = 96;

    switch (msg) {
    case WM_NCCREATE:
    {
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        break;
    }
    case WM_CREATE: {
        dpi = GetDpiForWindow(hwnd);

        CreateWindowW(L"STATIC", L"十六进制方向值：", WS_VISIBLE | WS_CHILD | SS_LEFTNOWORDWRAP, 0, 0, 0, 0, hwnd, (HMENU)IDC_STATIC_INPUT, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 0, 0, 0, 0, hwnd, (HMENU)IDC_INPUT, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"换算角度：", WS_VISIBLE | WS_CHILD | SS_LEFTNOWORDWRAP, 0, 0, 0, 0, hwnd, (HMENU)IDC_STATIC_OUTPUT, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY, 0, 0, 0, 0, hwnd, (HMENU)IDC_OUTPUT, nullptr, nullptr);

        ResizeControls(hwnd, dpi);
        RefreshFonts(hwnd, dpi);

        const int min_width = DpiScale(20 + 200 + 20, dpi);
        const int min_height = DpiScale(20 + 20 + 25 + 15 + 20 + 25 + 20, dpi);

        RECT rc = { 0, 0, min_width, min_height };
        AdjustWindowRectEx(&rc, GetWindowLong(hwnd, GWL_STYLE), FALSE, GetWindowLong(hwnd, GWL_EXSTYLE));
        SetWindowPos(hwnd, nullptr, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);

        break;
    }

    case WM_DPICHANGED:
    {
        dpi = HIWORD(wParam);
        RECT* prcNewWindow = (RECT*)lParam;

        SetWindowPos(hwnd, nullptr, prcNewWindow->left, prcNewWindow->top,
            prcNewWindow->right - prcNewWindow->left,
            prcNewWindow->bottom - prcNewWindow->top,
            SWP_NOZORDER | SWP_NOACTIVATE);

        ResizeControls(hwnd, dpi);
        RefreshFonts(hwnd, dpi);
        return 0;
    }

    case WM_ERASEBKGND:
    {
        HDC hdc = (HDC)wParam;
        RECT rect;
        GetClientRect(hwnd, &rect);
        FillRect(hdc, &rect, GetSysColorBrush(COLOR_WINDOW));
        return 1;
    }

    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
    {
        HDC hdc = (HDC)wParam;
        SetBkMode(hdc, TRANSPARENT);
        return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
    }

    case WM_COMMAND:
        if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_INPUT) {
            wchar_t input[64];
            GetWindowTextW(GetDlgItem(hwnd, IDC_INPUT), input, 64);
            std::wstring result = ConvertHexToAngle(input);
            SetWindowTextW(GetDlgItem(hwnd, IDC_OUTPUT), result.c_str());
        }
        break;

    case WM_DESTROY:
        if (hFont) DeleteObject(hFont);
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"HexToAngleWin";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;

    RegisterClass(&wc);

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"BY RYO",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
        nullptr, nullptr, hInst, nullptr
    );

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}