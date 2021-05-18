// helper.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <filesystem>
#include <cassert>
#include <map>
#include <vector>
#include <future>
#include <fstream>
#include <Windows.h>

std::pair<std::shared_ptr<void>, std::uint32_t> run_process(const std::filesystem::path& path, const std::wstring& parameters)
{
    TCHAR params[16 * MAX_PATH] = { 0 };
    std::copy(parameters.cbegin(), parameters.cend(), params);

    wchar_t desktop[] = L"WinSta0\\Default";
    STARTUPINFO si = { 0 };
    si.cb = sizeof(STARTUPINFO);
    si.lpDesktop = desktop;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = ::GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = ::GetStdHandle(STD_ERROR_HANDLE);

    DWORD dwCreationFlag = CREATE_UNICODE_ENVIRONMENT;

    wchar_t run_path[16 * MAX_PATH] = { 0 };
    lstrcpy(run_path, (L"\"" + path.wstring() + L"\" " + params).c_str());
    PROCESS_INFORMATION pi = { 0 };
    SECURITY_ATTRIBUTES saProcess = { 0 }, saThread = { 0 };
    saProcess.nLength = sizeof(saProcess);
    saProcess.lpSecurityDescriptor = NULL;
    saProcess.bInheritHandle = true;
    saThread.nLength = sizeof(saThread);
    saThread.lpSecurityDescriptor = NULL;
    saThread.bInheritHandle = true;

    auto ret = ::CreateProcess(nullptr, run_path, &saProcess, &saThread, false,
        CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi);
    ::CloseHandle(pi.hThread);
    if (ret)
        return{ std::shared_ptr<void>(pi.hProcess, [](void* p)
    {
        ::CloseHandle(p);
    }), pi.dwProcessId };
    else
        return{};
}

template < typename ValueType, typename CharT >
inline void split(std::vector<ValueType>& seq, const std::basic_string<CharT>& str, CharT separator)
{
    if (str.empty())
        return;

    std::basic_istringstream<CharT> iss(str);
    for (std::basic_string<CharT> s; std::getline(iss, s, separator); )
    {
        ValueType val;
        std::basic_istringstream<CharT> isss(s);

        isss >> val;

        assert(isss.good() || isss.eof());
        seq.emplace_back(std::move(val));
    }

    return;
}

const std::vector<std::pair<std::filesystem::path, std::vector<std::string>>> default_configs = {
    {"common", {"common"}},
    {"components/compute", {"compute_opencl", "compute_avx"}},
    {"components/io_device", {"io_rfsa_rfsg", "io_binary_file", "io_tdms", "io_uhd", "io_sync"}},
    {"components", {"render"}},
    {"business", {"rtsa", "ooda", "awg"}},
    {"framework", {"framework"}},
    {"third_party", {"protobuf"}},
};


std::vector<std::pair<std::filesystem::path, std::vector<std::string>>> load_config(const std::filesystem::path &path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.good())
        return default_configs;

    std::string buffer;
    in.seekg(0, std::ios_base::end);
    auto size = in.tellg();
    buffer.resize(size);
    in.seekg(0, std::ios_base::beg);

    in.read(buffer.data(), buffer.size());

    std::vector<std::string> lines;
    split(lines, buffer, '\n');

    std::vector<std::pair<std::filesystem::path, std::vector<std::string>>> configs;
    for (const auto& line : lines) {
        std::vector<std::string> key_value;
        split(key_value, line, ':');

        std::pair<std::filesystem::path, std::vector<std::string>> content;
        content.first = key_value[0];
        split(content.second, key_value[1], ',');

        configs.push_back(content);
    }

    return configs;

}

int wmain(int argc, wchar_t **argv)
{
    if (argc != 4) {
        std::cerr << "enter build type!" << std::endl;
        return -1;
    }

    auto path = std::filesystem::current_path();
    std::cout << path << std::endl;

    auto url{ std::filesystem::path{ argv[1] }.string() };
    std::wstring type{ argv[2] };
    std::string version{ std::filesystem::path{ argv[3] }.string() };

    auto git_paths = load_config("./config.dat");

    // clone or pull
    std::cout << std::endl << "clone or pull..." << std::endl;
    for (const auto& v : git_paths) {
        std::cout << "+---------------------+\n";
        std::cout << "|  " << v.first << "  |\n";
        if (v.first != "common" && v.first != "third_party" && v.first != "framework") {
            auto dir = path / v.first;
            if (!std::filesystem::exists(dir)) {
                std::filesystem::create_directories(dir);
                std::filesystem::current_path(dir);
                for (const auto& vv : v.second) {
                    std::cout << "|-----------------" << vv << "-----------------|\n";
                    std::cout << "|"; system(("git clone " + url + vv).c_str());
                    std::cout << "|"; system(("git checkout " + version).c_str());
                }
            }
            else {
                for (const auto& vv : v.second) {
                    auto path = dir / vv;
                    std::cout << "|-----------------" << vv << "-----------------|\n";
                    if (!std::filesystem::exists(path)) {
                        std::filesystem::current_path(dir);
                        system(("git clone " + url + vv).c_str());
                        std::filesystem::current_path(path);
                        system(("git checkout " + version).c_str());
                    }
                    else {
                        std::filesystem::current_path(path);
                        system(("git checkout " + version).c_str());
                        system("git pull");
                    }
                }
            }
        }
        else {
            if (v.first == "third_party") {
                auto vv = v.first.string();
                auto dir = path / vv;
                if (!std::filesystem::exists(dir)) {
                    std::filesystem::current_path(path);
                    system(("git clone " + url + vv).c_str());
                    std::filesystem::current_path(dir);
                    system(("git checkout " + version).c_str());
                }
                else {
                    std::cout << "|-----------------" << vv << "-----------------|\n";
                    std::filesystem::current_path(dir);
                    system(("git checkout " + version).c_str());
                    system("git pull");
                }
                continue;
            }
            for (const auto& vv : v.second) {
                std::cout << "|-----------------" << vv << "-----------------|\n";
                auto dir = path / vv;
                if (!std::filesystem::exists(dir)) {
                    std::filesystem::current_path(path);
                    system(("git clone " + url + vv).c_str());
                    std::filesystem::current_path(dir);
                    system(("git checkout " + version).c_str());
                }
                else {
                    std::filesystem::current_path(dir);
                    system(("git checkout " + version).c_str());
                    system("git pull");
                }
            }
        }
    }


    
    // build
    std::cout << std::endl << "build..." << std::endl;
    char* param = nullptr;
    std::size_t param_len = 0;
    _dupenv_s(&param, &param_len, "VS2019INSTALLDIR");
    if (param_len == 0)
    {
        std::cerr << "not found VS directory..." << std::endl;
        return -1;
    }

    std::filesystem::path msvc_path = std::string{ param, param_len - 1};
    auto vs_dev_path = msvc_path / "Common7/Tools/VsDevCmd.bat";
    auto cmd = "%comspec% /k \"" + vs_dev_path.string() + "\"";
    system(("\"" + vs_dev_path.string() + "\"").c_str());
    
    for (const auto& v : git_paths) {
        for (const auto& vv : v.second) {
            auto project_path = path / v.first;
            if (v.first == "third_party") {
                if(vv == "protobuf")
                    project_path = project_path / "src/protobuf/cmake/out/protobuf.sln";
            }
            else if (v.first != "common" && v.first != "framework")
                project_path = project_path / vv / (vv + ".sln");
            else
                project_path = project_path / (vv + ".sln");

            std::cout << "compile " << project_path << std::endl;
            auto ret = run_process(msvc_path / "Common7/IDE/devenv.com", project_path.wstring() + L" /Rebuild \"" + type + L"|X64\"");
        
            ::WaitForSingleObject(ret.first.get(), INFINITE);
        }
    }

    system("pause");
    return 0;
}
