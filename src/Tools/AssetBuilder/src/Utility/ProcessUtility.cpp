#include "ProcessUtility.h"
#include <format>
#include <Windows.h>

#include "AtlasTrace/Logging.h"

namespace
{
    struct SafeHandlePair
    {
        SafeHandlePair() = default;

        // ReSharper disable once CppParameterMayBeConst
        SafeHandlePair(HANDLE readHandle, HANDLE writeHandle) : m_ReadHandle(readHandle), m_WriteHandle(writeHandle)
        {
        }

        SafeHandlePair(const SafeHandlePair&) = delete;

        SafeHandlePair(SafeHandlePair&& other) noexcept
        {
            m_ReadHandle = other.m_ReadHandle;
            m_WriteHandle = other.m_WriteHandle;
            other.m_ReadHandle = INVALID_HANDLE_VALUE;
            other.m_WriteHandle = INVALID_HANDLE_VALUE;
        }

        ~SafeHandlePair()
        {
            CloseReadHandle();
            CloseWriteHandle();
        }

        void CloseReadHandle()
        {
            if (m_ReadHandle != INVALID_HANDLE_VALUE)
            {
                CloseHandle(m_ReadHandle);
                m_ReadHandle = INVALID_HANDLE_VALUE;
            }
        }

        void CloseWriteHandle()
        {
            if (m_WriteHandle != INVALID_HANDLE_VALUE)
            {
                CloseHandle(m_WriteHandle);
                m_WriteHandle = INVALID_HANDLE_VALUE;
            }
        }

        HANDLE m_ReadHandle = INVALID_HANDLE_VALUE;
        HANDLE m_WriteHandle = INVALID_HANDLE_VALUE;
    };

    int executeChildProcess(const std::string& executable, const std::string& args, const SafeHandlePair& stdOutHandles,
                             const SafeHandlePair& stdErrHandles)
    {
        PROCESS_INFORMATION piProcInfo;
        STARTUPINFOA siStartInfo;

        ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
        ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
        siStartInfo.cb = sizeof(STARTUPINFO);
        siStartInfo.hStdOutput = stdOutHandles.m_WriteHandle;
        siStartInfo.hStdError = stdErrHandles.m_WriteHandle;
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

        // Create the child process.

        std::string mutableArgs = args;

        const BOOL success = CreateProcessA(
            executable.c_str(),
            mutableArgs.data(),
            nullptr,
            nullptr,
            TRUE,
            0,
            nullptr,
            nullptr,
            &siStartInfo,
            &piProcInfo);

        DWORD exitCode = -1;

        // If an error occurs, exit the application.
        if (! success)
        {
            AT_ERROR(ProcessUtility, "CreateProcess failed ({}).\n", GetLastError());
            exitCode = -1;
        }
        else
        {
            // Close handles to the child process and its primary thread.
            // Some applications might keep these handles to monitor the status
            // of the child process, for example.

            do
            {
                if (FALSE == GetExitCodeProcess(piProcInfo.hProcess, &exitCode))
                {
                    AT_ERROR(ProcessUtility, "GetExitCodeProcess failed");
                    exitCode = -1;
                }
            } while(STILL_ACTIVE == exitCode);

            CloseHandle(piProcInfo.hProcess);
            CloseHandle(piProcInfo.hThread);
        }

        return static_cast<int>(exitCode);
    }

    std::string readFromPipe(const SafeHandlePair& handles)
    {
        constexpr int c_bufferSize = 4096;

        DWORD dwRead;
        CHAR chBuf[c_bufferSize];

        std::string output;
        for (;;)
        {
            const BOOL success = ReadFile(handles.m_ReadHandle, chBuf, c_bufferSize, &dwRead, nullptr);
            if (! success || dwRead == 0)
            {
                break;
            }

            output += std::string_view(chBuf, dwRead);
        }

        return output;
    }

    SafeHandlePair createPipes()
    {
        SECURITY_ATTRIBUTES saAttr;
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = nullptr;

        HANDLE readHandle, writeHandle;

        if (! CreatePipe(&readHandle, &writeHandle, &saAttr, 0))
        {
            AT_ERROR(ProcessUtilty, "CreatePipe failed. Error: {}", GetLastError());
            return {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
        }

        if (! SetHandleInformation(readHandle, HANDLE_FLAG_INHERIT, 0))
        {
            AT_ERROR(ProcessUtilty, "SetHandleInformation failed. Error: {}", GetLastError());
            return {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
        }

        return {readHandle, writeHandle};
    }
}

int asset_builder::utility::process_utility::execute(const std::string& executable, const std::string& args,
                                                     std::string& stdOut, std::string& stdErr)
{
    SafeHandlePair stdOutPipe = createPipes();
    SafeHandlePair stdErrPipe = createPipes();

    const int exitCode = executeChildProcess(executable, args, stdOutPipe, stdErrPipe);
    stdOutPipe.CloseWriteHandle();
    stdErrPipe.CloseWriteHandle();

    stdOut = readFromPipe(stdOutPipe);
    stdErr = readFromPipe(stdErrPipe);

    return exitCode;
}
