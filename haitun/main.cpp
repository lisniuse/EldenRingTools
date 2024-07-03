#include <windows.h>
#include <fstream>
#include <string>
#include <sstream>
#include <shlobj.h> // For ShellExecute and SHGetFolderPath
#include <Lmcons.h> // For UNLEN

#define ID_STEAMID      101
#define ID_PASSWORD     102
#define ID_NAME         103
#define ID_SAVE         104
#define ID_OPEN_FOLDER  105

HFONT hFont; // 用于保存字体句柄

/**
 * 检查文件是否存在
 * 
 * @param filePath 文件路径
 * @return 如果文件存在则返回 true，否则返回 false
 */
bool FileExists(const std::string& filePath) {
    std::ifstream file(filePath.c_str());
    return file.is_open();
}

/**
 * 从文件中读取内容
 * 
 * @param filePath 文件路径
 * @param content 输出的文件内容
 * @return 如果成功读取文件内容则返回 true，否则返回 false
 */
bool ReadFileContent(const std::string& filePath, std::string& content) {
    std::ifstream file(filePath.c_str());
    if (!file.is_open()) {
        return false;
    }
    std::ostringstream oss;
    oss << file.rdbuf();
    content = oss.str();
    return true;
}

/**
 * 从INI文件中读取联机密码
 * 
 * @param filePath INI文件路径
 * @param password 输出的联机密码
 * @return 如果成功读取联机密码则返回 true，否则返回 false
 */
bool ReadPasswordFromIni(const std::string& filePath, std::string& password) {
    std::ifstream file(filePath.c_str());
    if (!file.is_open()) {
        return false;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::string::size_type pos = line.find("cooppassword = ");
        if (pos != std::string::npos) {
            password = line.substr(pos + 15); // 15 是 "cooppassword = " 的长度
            return true;
        }
    }
    return false;
}

/**
 * 将内容写入到文件中
 * 
 * @param filePath 文件路径
 * @param content 要写入的内容
 * @return 如果成功写入内容则返回 true，否则返回 false
 */
bool WriteFileContent(const std::string& filePath, const std::string& content) {
    std::ofstream file(filePath.c_str());
    if (!file.is_open()) {
        return false;
    }
    file << content;
    return true;
}

/**
 * 更新INI文件中的联机密码
 * 
 * @param filePath INI文件路径
 * @param newPassword 要更新的联机密码
 * @return 如果成功更新联机密码则返回 true，否则返回 false
 */
bool UpdatePasswordInIni(const std::string& filePath, const std::string& newPassword) {
    std::ifstream file(filePath.c_str());
    if (!file.is_open()) {
        return false;
    }
    
    std::stringstream buffer;
    std::string line;
    bool found = false;
    
    while (std::getline(file, line)) {
        if (line.find("cooppassword = ") == 0) {
            buffer << "cooppassword = " << newPassword << "\n";
            found = true;
        } else {
            buffer << line << "\n";
        }
    }
    
    if (!found) {
        return false;
    }
    
    std::ofstream outFile(filePath.c_str(), std::ios::trunc);
    if (!outFile.is_open()) {
        return false;
    }
    
    outFile << buffer.str();
    return true;
}

/**
 * 获取当前用户的用户名
 * 
 * @return 用户名字符串
 */
std::string GetCurrentUserName() {
    char userName[UNLEN + 1];
    DWORD size = sizeof(userName);
    if (GetUserName(userName, &size)) {
        return std::string(userName);
    } else {
        return "";
    }
}

/**
 * 窗口过程函数，用于处理窗口消息
 * 
 * @param hwnd 窗口句柄
 * @param Message 消息标识符
 * @param wParam 附加消息参数
 * @param lParam 附加消息参数
 * @return 返回消息处理结果
 */
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    static HWND hSteamID, hPassword, hName, hSaveButton, hOpenFolderButton;
    static HWND hLabelSteamID, hLabelPassword, hLabelName;
    static HBRUSH hbrBackground;

    switch (Message) {
        case WM_CREATE: {
            // 创建并设置字体
            hFont = CreateFont(
                20,                        // 字体高度
                0,                         // 字符宽度（0表示自动计算）
                0,                         // 字体倾斜角度
                0,                         // 基线倾斜角度
                FW_NORMAL,                 // 字体粗细
                FALSE,                     // 是否斜体
                FALSE,                     // 是否下划线
                FALSE,                     // 是否删除线
                DEFAULT_CHARSET,           // 字符集
                OUT_DEFAULT_PRECIS,        // 输出精度
                CLIP_DEFAULT_PRECIS,       // 剪裁精度
                DEFAULT_QUALITY,           // 质量
                DEFAULT_PITCH | FF_SWISS,  // 字体间距和字体族
                "微软雅黑");              // 字体名称

            // 创建标签控件
            hLabelSteamID = CreateWindow("STATIC", "SteamID:", WS_VISIBLE | WS_CHILD | SS_LEFT | SS_CENTERIMAGE,
                                         10, 10, 100, 30, hwnd, NULL, NULL, NULL);
            hLabelPassword = CreateWindow("STATIC", "联机密码:", WS_VISIBLE | WS_CHILD | SS_LEFT | SS_CENTERIMAGE,
                                           10, 50, 100, 30, hwnd, NULL, NULL, NULL);
            hLabelName = CreateWindow("STATIC", "玩家名称:", WS_VISIBLE | WS_CHILD | SS_LEFT | SS_CENTERIMAGE,
                                       10, 90, 100, 30, hwnd, NULL, NULL, NULL);

            // 创建输入框控件
            hSteamID = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                                    120, 10, 200, 30, hwnd, (HMENU)ID_STEAMID, NULL, NULL);
            hPassword = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                                     120, 50, 200, 30, hwnd, (HMENU)ID_PASSWORD, NULL, NULL);
            hName = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                                 120, 90, 200, 30, hwnd, (HMENU)ID_NAME, NULL, NULL);

            // 创建按钮控件
            hSaveButton = CreateWindow("BUTTON", "保存", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_DEFPUSHBUTTON,
                                       10, 130, 310, 40, hwnd, (HMENU)ID_SAVE, NULL, NULL);
            hOpenFolderButton = CreateWindow("BUTTON", "打开存档目录", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                             330, 10, 100, 30, hwnd, (HMENU)ID_OPEN_FOLDER, NULL, NULL);

            // 设置字体到控件
            SendMessage(hLabelSteamID, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLabelPassword, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLabelName, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hSteamID, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hPassword, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hName, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hSaveButton, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hOpenFolderButton, WM_SETFONT, (WPARAM)hFont, TRUE);

            // 创建背景画刷
            hbrBackground = CreateSolidBrush(GetSysColor(COLOR_WINDOW));

            // 读取文件内容并填充到编辑框
            char exePath[MAX_PATH];
            GetModuleFileName(NULL, exePath, MAX_PATH);
            std::string exeDir = std::string(exePath);
            size_t pos = exeDir.find_last_of("\\/");
            exeDir = exeDir.substr(0, pos + 1);

            std::string steamIDFile = exeDir + "steam_settings\\force_steamid.txt";
            std::string passwordFile = exeDir + "SeamlessCoop\\ersc_settings.ini";
            std::string nameFile = exeDir + "steam_settings\\force_account_name.txt";

            std::string steamID, password, name;
            if (ReadFileContent(steamIDFile, steamID)) {
                SetWindowText(hSteamID, steamID.c_str());
            }
            if (ReadPasswordFromIni(passwordFile, password)) {
                SetWindowText(hPassword, password.c_str());
            }
            if (ReadFileContent(nameFile, name)) {
                SetWindowText(hName, name.c_str());
            }

            break;
        }

        case WM_CTLCOLORSTATIC: {
            // 设置标签控件的背景颜色
            if ((HWND)lParam == hLabelSteamID || (HWND)lParam == hLabelPassword || (HWND)lParam == hLabelName) {
                HDC hdcStatic = (HDC)wParam;
                SetBkColor(hdcStatic, GetSysColor(COLOR_WINDOW));
                SetTextColor(hdcStatic, GetSysColor(COLOR_WINDOWTEXT));
                return (LRESULT)hbrBackground;
            }
            break;
        }

        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_SAVE) {
                // 获取输入框内容
                char steamID[256], password[256], name[256];
                GetWindowText(hSteamID, steamID, sizeof(steamID));
                GetWindowText(hPassword, password, sizeof(password));
                GetWindowText(hName, name, sizeof(name));

                // 获取当前 exe 的路径
                char exePath[MAX_PATH];
                GetModuleFileName(NULL, exePath, MAX_PATH);
                std::string exeDir = std::string(exePath);
                size_t pos = exeDir.find_last_of("\\/");
                exeDir = exeDir.substr(0, pos + 1);

                // 文件路径
                std::string steamIDFile = exeDir + "steam_settings\\force_steamid.txt";
                std::string passwordFile = exeDir + "SeamlessCoop\\ersc_settings.ini";
                std::string nameFile = exeDir + "steam_settings\\force_account_name.txt";

                bool success = true;

                // 保存文件
                if (!WriteFileContent(steamIDFile, steamID)) {
                    success = false;
                }
                if (!WriteFileContent(nameFile, name)) {
                    success = false;
                }
                if (!UpdatePasswordInIni(passwordFile, password)) {
                    success = false;
                }

                // 显示保存成功的图标
                if (success) {
                    MessageBox(hwnd, "保存成功", "成功", MB_ICONINFORMATION | MB_OK);
                } else {
                    MessageBox(hwnd, "请先安装联机MOD和学习版补丁", "错误", MB_ICONEXCLAMATION | MB_OK);
                }
            } else if (LOWORD(wParam) == ID_OPEN_FOLDER) {
                // 获取当前用户的用户名
                std::string userName = GetCurrentUserName();
                if (userName.empty()) {
                    MessageBox(hwnd, "无法获取用户名", "错误", MB_ICONEXCLAMATION | MB_OK);
                    break;
                }

                // 构建文件夹路径
                std::string folderPath = "C:\\Users\\" + userName + "\\AppData\\Roaming\\EldenRing";

                // 打开文件夹
                ShellExecute(NULL, "explore", folderPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
            }
            break;
        }

        case WM_DESTROY: {
            // 清理资源
            if (hFont) {
                DeleteObject(hFont);
            }
            if (hbrBackground) {
                DeleteObject(hbrBackground);
            }
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProc(hwnd, Message, wParam, lParam);
    }
    return 0;
}

/**
 * WinMain 函数是程序的入口点
 * 
 * @param hInstance 当前实例句柄
 * @param hPrevInstance 之前的实例句柄（通常为 NULL）
 * @param lpCmdLine 命令行参数
 * @param nCmdShow 窗口显示状态
 * @return 返回值
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc; /* 窗口类结构体 */
    HWND hwnd; /* 窗口句柄 */
    MSG msg; /* 消息结构体 */

    // 获取当前 exe 的路径
    char exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);
    std::string exeDir = std::string(exePath);
    size_t pos = exeDir.find_last_of("\\/");
    exeDir = exeDir.substr(0, pos + 1);

    // 文件路径
    std::string steamIDFile = exeDir + "steam_settings\\force_steamid.txt";
    std::string passwordFile = exeDir + "SeamlessCoop\\ersc_settings.ini";
    std::string nameFile = exeDir + "steam_settings\\force_account_name.txt";

    // 检查文件是否存在
    if (!FileExists(steamIDFile) || !FileExists(passwordFile) || !FileExists(nameFile)) {
        MessageBox(NULL, "请先安装联机MOD和学习版补丁", "错误", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "WindowClass";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // 计算窗口居中位置
    RECT rect;
    int width = 450; // 增加宽度以容纳新按钮
    int height = 220;

    // 获取显示屏的大小
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
    int screenWidth = rect.right;
    int screenHeight = rect.bottom;

    int x = (screenWidth - width) / 2;
    int y = (screenHeight - height) / 2;

    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "WindowClass", "【法环】快捷修改无缝联机信息工具 v1.0.0",
                          WS_VISIBLE | WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX, // 禁用调整大小和最大化
                          x, y, width, height,
                          NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

