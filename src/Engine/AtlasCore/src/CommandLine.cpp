#include "AtlasCorePCH.h"
#include "CommandLine.h"

#include <format>
#include <iostream>
#include <sstream>
#include <string>

#include "AtlasTrace/Logging.h"

bool atlas::core::command_line::TypedArgReader<bool>::Read(const std::string& value, void* pTargetVariable)
{
    const auto typedTarget = static_cast<bool*>(pTargetVariable);
    if (value == "false" || value == "f" || value == "FALSE" || value == "0")
    {
        *typedTarget = false;
    }
    else
    {
        *typedTarget = true;
    }

    return true;
}

bool atlas::core::command_line::TypedArgReader<int32_t>::Read(const std::string& value, void* pTargetVariable)
{
    const auto typedTarget = static_cast<int32_t*>(pTargetVariable);
    *typedTarget = std::stoi(value);
    return false;
}

bool atlas::core::command_line::TypedArgReader<float>::Read(const std::string& value, void* pTargetVariable)
{
    const auto typedTarget = static_cast<float*>(pTargetVariable);
    *typedTarget = std::stof(value);
    return false;
}

bool atlas::core::command_line::TypedArgReader<std::string>::Read(const std::string& value, void* pTargetVariable)
{
    const auto typedTarget = static_cast<std::string*>(pTargetVariable);
    *typedTarget = value;
    return true;
}

bool atlas::core::command_line::TypedArgReader<std::filesystem::path>::Read(const std::string& value, void* pTargetVariable)
{
    const auto typedTarget = static_cast<std::filesystem::path*>(pTargetVariable);
    *typedTarget = value;
    return true;
}

bool atlas::core::command_line::TypedArgReader<std::vector<std::string>>::Read(const std::string& value, void* pTargetVariable)
{
    const auto typedTarget = static_cast<std::vector<std::string>*>(pTargetVariable);

    std::vector<std::string> values;
    std::string segment;
    std::stringstream test(value);
    while(std::getline(test, segment, ';'))
    {
        values.push_back(segment);
    }

    *typedTarget = values;
    return true;
}

bool atlas::core::command_line::CommandLineManager::TryRead(const int argc, char** argv)
{
    bool bIsError = false;
    std::vector<std::string> errors;

    int iParameterIdx = 1;
    while(iParameterIdx < argc)
    {
        std::string parameter = argv[iParameterIdx];

        if (parameter.empty())
        {
            ++iParameterIdx;
            continue;
        }

        Argument* currentField = nullptr;
        for(Argument& field : m_Arguments)
        {
            if ((parameter.size() > 1 && field.m_Switch == parameter[1]) || std::format("--{}", field.m_SwitchLong) == parameter)
            {
                currentField = &field;
                break;
            }
        }

        ++iParameterIdx;
        if (currentField == nullptr)
        {
            AT_ERROR(CommandLine, "Unknown option: {}", parameter);
            bIsError = true;
            while(iParameterIdx < argc && argv[iParameterIdx][0] != '-')
            {
                ++iParameterIdx;
            }
        }
        else
        {
            const char* szValueEntry = iParameterIdx < (argc) ? argv[iParameterIdx] : nullptr;

            if (szValueEntry == nullptr || szValueEntry[0] == '-')
            {
                // Special handling for "flag fields" (bool), which can be set just by the presence of the flag
                if (currentField->m_bIsFlag)
                {
                    if (currentField->m_ReadFunc({}, currentField->m_TargetVariable))
                    {
                        currentField->m_bIsSet = true;
                    }
                }
                else
                {
                    AT_ERROR(CommandLine, "Parameter {} had no value!", currentField->m_Name);
                    bIsError = true;
                }

                while(iParameterIdx < argc && argv[iParameterIdx][0] != '-')
                {
                    ++iParameterIdx;
                }
                continue;
            }

            ++iParameterIdx;
            if (!currentField->m_ReadFunc(szValueEntry, currentField->m_TargetVariable))
            {
                AT_ERROR(CommandLine, "Parameter {} failed to read value {}", currentField->m_Name, szValueEntry);
                bIsError = true;
            }
            else
            {
                currentField->m_bIsSet = true;
            }
        }
    }

    for(const Argument& field : m_Arguments)
    {
        if (field.m_bIsRequired && !field.m_bIsSet)
        {
            errors.push_back(std::format("Required parameter {} was not set", field.m_Name));
            bIsError = true;
        }
    }

    if (!errors.empty())
    {
        if (bIsError)
        {
            std::cerr << "Argument parsing encountered some errors\n";
        }
        else
        {
            std::cout << "Argument parsing encountered some warnings\n";
        }

        for(const auto& error : errors)
        {
            std::cout << "    " << error << "\n";
        }
    }

    return !bIsError;
}
