// Standalone launcher: Win32 + statically linked MSVC runtime, no Qt required.
#define UNICODE
#define _UNICODE
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <stdexcept>

static std::wstring setting(const std::wstring &file, const wchar_t *section, const wchar_t *key)
{
    wchar_t value[512] = {};
    GetPrivateProfileStringW(section, key, L"", value, 512, file.c_str());
    return value;
}

static bool exists(const std::wstring &path)
{
    const DWORD attributes = GetFileAttributesW(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

static void output(const std::wstring &text, DWORD stream)
{
    const int length = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::vector<char> bytes(length);
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, bytes.data(), length, nullptr, nullptr);
    DWORD written = 0;
    WriteFile(GetStdHandle(stream), bytes.data(), length - 1, &written, nullptr);
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    bool check = false;
    try {
        int argc = 0;
        LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (!argv) throw std::wstring(L"Cannot read command line.");
        std::vector<std::wstring> args;
        for (int i = 1; i < argc; ++i) args.emplace_back(argv[i]);
        LocalFree(argv);
        for (const auto &arg : args) if (arg == L"-Check") check = true;
        bool custom = false;
        std::wstring version;
        for (size_t i = 0; i < args.size(); ++i) {
            if (args[i] == L"-Check") continue;
            if (args[i] == L"-Custom") custom = true;
            else if (args[i] == L"-Version" && i + 1 < args.size()) version = args[++i];
            else throw std::wstring(L"Usage: ZIMA-CAD-Parts.exe [-Version NAME] [-Custom] [-Check]");
        }
        wchar_t module[32768];
        DWORD count = GetModuleFileNameW(nullptr, module, 32768);
        if (!count || count >= 32768) throw std::wstring(L"Cannot locate launcher.");
        std::wstring root(module, count);
        root = root.substr(0, root.find_last_of(L"\\/"));
        if (version.empty()) {
            version = setting(root + L"\\launcher.ini", L"launcher", L"windows");
            custom = setting(root + L"\\launcher.ini", L"launcher", L"windows_custom") == L"true";
        }
        const auto alphaNumeric = [](wchar_t c) {
            return (c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z') || (c >= L'0' && c <= L'9');
        };
        bool valid = !version.empty();
        if (custom) {
            valid = valid && alphaNumeric(version.front());
            for (wchar_t c : version) valid = valid && (alphaNumeric(c) || c == L'-' || c == L'_' || c == L'.');
            valid = valid && version.back() != L'.';
        } else {
            valid = version.size() == 10;
            for (wchar_t c : version) valid = valid && c >= L'0' && c <= L'9';
        }
        if (!valid) throw std::wstring(L"Invalid build name in launcher.ini or command line.");
        const std::wstring directory = root + (custom ? L"\\custom\\windows\\" : L"\\windows\\") + version;
        const std::wstring exe = directory + L"\\ZIMA-CAD-Parts.exe";
        if (!exists(exe)) throw std::wstring(L"Build not found: ") + exe;
        if (!custom) {
            const auto manifest = directory + L"\\build.ini";
            if (setting(manifest, L"build", L"version") != version ||
                setting(manifest, L"build", L"platform") != L"windows-x64")
                throw std::wstring(L"Build manifest does not match the selected version.");
        }
        if (check) { output(exe + L"\n", STD_OUTPUT_HANDLE); return 0; }
        for (const wchar_t *name : {L"QT_PLUGIN_PATH", L"QT_QPA_PLATFORM_PLUGIN_PATH", L"QML_IMPORT_PATH", L"QML2_IMPORT_PATH", L"QTWEBENGINEPROCESS_PATH", L"QTWEBENGINE_RESOURCES_PATH", L"QTWEBENGINE_LOCALES_PATH"})
            SetEnvironmentVariableW(name, nullptr);
        const wchar_t *resources[][2] = {
            {L"CSF_ShadersDirectory", L"Shaders"}, {L"CSF_SHMessage", L"SHMessage"},
            {L"CSF_XSMessage", L"XSMessage"}, {L"CSF_STEPDefaults", L"XSTEPResource"},
            {L"CSF_IGESDefaults", L"XSTEPResource"}, {L"CSF_PluginDefaults", L"StdResource"},
            {L"CSF_StandardDefaults", L"StdResource"}, {L"CSF_XCAFDefaults", L"StdResource"},
            {L"CSF_MDTVTexturesDirectory", L"Textures"}, {L"CSF_XmlOcafResource", L"XmlOcafResource"}
        };
        for (const auto &resource : resources) {
            const auto path = directory + L"\\occt\\" + resource[1];
            SetEnvironmentVariableW(resource[0], path.c_str());
        }
        wchar_t system[32768], windows[32768];
        GetSystemDirectoryW(system, 32768);
        GetWindowsDirectoryW(windows, 32768);
        const auto searchPath = directory + L";" + system + L";" + windows;
        SetEnvironmentVariableW(L"PATH", searchPath.c_str());
        std::wstring command = L"\"" + exe + L"\"";
        STARTUPINFOW startup = {};
        startup.cb = sizeof(startup);
        PROCESS_INFORMATION process = {};
        if (!CreateProcessW(exe.c_str(), command.data(), nullptr, nullptr, FALSE, 0, nullptr, directory.c_str(), &startup, &process))
            throw std::wstring(L"Cannot start selected build. Windows error: ") + std::to_wstring(GetLastError());
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
        return 0;
    } catch (const std::wstring &error) {
        output(error + L"\n", STD_ERROR_HANDLE);
        if (!check) MessageBoxW(nullptr, error.c_str(), L"ZIMA-CAD-Parts", MB_OK | MB_ICONERROR);
        return 1;
    }
}
