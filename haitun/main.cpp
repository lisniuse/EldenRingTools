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

HFONT hFont; // ���ڱ���������

/**
 * ����ļ��Ƿ����
 * 
 * @param filePath �ļ�·��
 * @return ����ļ������򷵻� true�����򷵻� false
 */
bool FileExists(const std::string& filePath) {
    std::ifstream file(filePath.c_str());
    return file.is_open();
}

/**
 * ���ļ��ж�ȡ����
 * 
 * @param filePath �ļ�·��
 * @param content ������ļ�����
 * @return ����ɹ���ȡ�ļ������򷵻� true�����򷵻� false
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
 * ��INI�ļ��ж�ȡ��������
 * 
 * @param filePath INI�ļ�·��
 * @param password �������������
 * @return ����ɹ���ȡ���������򷵻� true�����򷵻� false
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
            password = line.substr(pos + 15); // 15 �� "cooppassword = " �ĳ���
            return true;
        }
    }
    return false;
}

/**
 * ������д�뵽�ļ���
 * 
 * @param filePath �ļ�·��
 * @param content Ҫд�������
 * @return ����ɹ�д�������򷵻� true�����򷵻� false
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
 * ����INI�ļ��е���������
 * 
 * @param filePath INI�ļ�·��
 * @param newPassword Ҫ���µ���������
 * @return ����ɹ��������������򷵻� true�����򷵻� false
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
 * ��ȡ��ǰ�û����û���
 * 
 * @return �û����ַ���
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
 * ���ڹ��̺��������ڴ�������Ϣ
 * 
 * @param hwnd ���ھ��
 * @param Message ��Ϣ��ʶ��
 * @param wParam ������Ϣ����
 * @param lParam ������Ϣ����
 * @return ������Ϣ������
 */
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    static HWND hSteamID, hPassword, hName, hSaveButton, hOpenFolderButton;
    static HWND hLabelSteamID, hLabelPassword, hLabelName;
    static HBRUSH hbrBackground;

    switch (Message) {
        case WM_CREATE: {
            // ��������������
            hFont = CreateFont(
                20,                        // ����߶�
                0,                         // �ַ���ȣ�0��ʾ�Զ����㣩
                0,                         // ������б�Ƕ�
                0,                         // ������б�Ƕ�
                FW_NORMAL,                 // �����ϸ
                FALSE,                     // �Ƿ�б��
                FALSE,                     // �Ƿ��»���
                FALSE,                     // �Ƿ�ɾ����
                DEFAULT_CHARSET,           // �ַ���
                OUT_DEFAULT_PRECIS,        // �������
                CLIP_DEFAULT_PRECIS,       // ���þ���
                DEFAULT_QUALITY,           // ����
                DEFAULT_PITCH | FF_SWISS,  // �������������
                "΢���ź�");              // ��������

            // ������ǩ�ؼ�
            hLabelSteamID = CreateWindow("STATIC", "SteamID:", WS_VISIBLE | WS_CHILD | SS_LEFT | SS_CENTERIMAGE,
                                         10, 10, 100, 30, hwnd, NULL, NULL, NULL);
            hLabelPassword = CreateWindow("STATIC", "��������:", WS_VISIBLE | WS_CHILD | SS_LEFT | SS_CENTERIMAGE,
                                           10, 50, 100, 30, hwnd, NULL, NULL, NULL);
            hLabelName = CreateWindow("STATIC", "�������:", WS_VISIBLE | WS_CHILD | SS_LEFT | SS_CENTERIMAGE,
                                       10, 90, 100, 30, hwnd, NULL, NULL, NULL);

            // ���������ؼ�
            hSteamID = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                                    120, 10, 200, 30, hwnd, (HMENU)ID_STEAMID, NULL, NULL);
            hPassword = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                                     120, 50, 200, 30, hwnd, (HMENU)ID_PASSWORD, NULL, NULL);
            hName = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                                 120, 90, 200, 30, hwnd, (HMENU)ID_NAME, NULL, NULL);

            // ������ť�ؼ�
            hSaveButton = CreateWindow("BUTTON", "����", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_DEFPUSHBUTTON,
                                       10, 130, 310, 40, hwnd, (HMENU)ID_SAVE, NULL, NULL);
            hOpenFolderButton = CreateWindow("BUTTON", "�򿪴浵Ŀ¼", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                             330, 10, 100, 30, hwnd, (HMENU)ID_OPEN_FOLDER, NULL, NULL);

            // �������嵽�ؼ�
            SendMessage(hLabelSteamID, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLabelPassword, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLabelName, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hSteamID, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hPassword, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hName, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hSaveButton, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hOpenFolderButton, WM_SETFONT, (WPARAM)hFont, TRUE);

            // ����������ˢ
            hbrBackground = CreateSolidBrush(GetSysColor(COLOR_WINDOW));

            // ��ȡ�ļ����ݲ���䵽�༭��
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
            // ���ñ�ǩ�ؼ��ı�����ɫ
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
                // ��ȡ���������
                char steamID[256], password[256], name[256];
                GetWindowText(hSteamID, steamID, sizeof(steamID));
                GetWindowText(hPassword, password, sizeof(password));
                GetWindowText(hName, name, sizeof(name));

                // ��ȡ��ǰ exe ��·��
                char exePath[MAX_PATH];
                GetModuleFileName(NULL, exePath, MAX_PATH);
                std::string exeDir = std::string(exePath);
                size_t pos = exeDir.find_last_of("\\/");
                exeDir = exeDir.substr(0, pos + 1);

                // �ļ�·��
                std::string steamIDFile = exeDir + "steam_settings\\force_steamid.txt";
                std::string passwordFile = exeDir + "SeamlessCoop\\ersc_settings.ini";
                std::string nameFile = exeDir + "steam_settings\\force_account_name.txt";

                bool success = true;

                // �����ļ�
                if (!WriteFileContent(steamIDFile, steamID)) {
                    success = false;
                }
                if (!WriteFileContent(nameFile, name)) {
                    success = false;
                }
                if (!UpdatePasswordInIni(passwordFile, password)) {
                    success = false;
                }

                // ��ʾ����ɹ���ͼ��
                if (success) {
                    MessageBox(hwnd, "����ɹ�", "�ɹ�", MB_ICONINFORMATION | MB_OK);
                } else {
                    MessageBox(hwnd, "���Ȱ�װ����MOD��ѧϰ�油��", "����", MB_ICONEXCLAMATION | MB_OK);
                }
            } else if (LOWORD(wParam) == ID_OPEN_FOLDER) {
                // ��ȡ��ǰ�û����û���
                std::string userName = GetCurrentUserName();
                if (userName.empty()) {
                    MessageBox(hwnd, "�޷���ȡ�û���", "����", MB_ICONEXCLAMATION | MB_OK);
                    break;
                }

                // �����ļ���·��
                std::string folderPath = "C:\\Users\\" + userName + "\\AppData\\Roaming\\EldenRing";

                // ���ļ���
                ShellExecute(NULL, "explore", folderPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
            }
            break;
        }

        case WM_DESTROY: {
            // ������Դ
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
 * WinMain �����ǳ������ڵ�
 * 
 * @param hInstance ��ǰʵ�����
 * @param hPrevInstance ֮ǰ��ʵ�������ͨ��Ϊ NULL��
 * @param lpCmdLine �����в���
 * @param nCmdShow ������ʾ״̬
 * @return ����ֵ
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc; /* ������ṹ�� */
    HWND hwnd; /* ���ھ�� */
    MSG msg; /* ��Ϣ�ṹ�� */

    // ��ȡ��ǰ exe ��·��
    char exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);
    std::string exeDir = std::string(exePath);
    size_t pos = exeDir.find_last_of("\\/");
    exeDir = exeDir.substr(0, pos + 1);

    // �ļ�·��
    std::string steamIDFile = exeDir + "steam_settings\\force_steamid.txt";
    std::string passwordFile = exeDir + "SeamlessCoop\\ersc_settings.ini";
    std::string nameFile = exeDir + "steam_settings\\force_account_name.txt";

    // ����ļ��Ƿ����
    if (!FileExists(steamIDFile) || !FileExists(passwordFile) || !FileExists(nameFile)) {
        MessageBox(NULL, "���Ȱ�װ����MOD��ѧϰ�油��", "����", MB_ICONEXCLAMATION | MB_OK);
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

    // ���㴰�ھ���λ��
    RECT rect;
    int width = 450; // ���ӿ���������°�ť
    int height = 220;

    // ��ȡ��ʾ���Ĵ�С
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
    int screenWidth = rect.right;
    int screenHeight = rect.bottom;

    int x = (screenWidth - width) / 2;
    int y = (screenHeight - height) / 2;

    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "WindowClass", "������������޸��޷�������Ϣ���� v1.0.0",
                          WS_VISIBLE | WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX, // ���õ�����С�����
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

