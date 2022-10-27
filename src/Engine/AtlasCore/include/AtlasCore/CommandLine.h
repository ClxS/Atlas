#pragma once
#include <filesystem>

namespace atlas::core::command_line
{
    template<typename TReadType>
    struct TypedArgReader;

#define DEFINE_ARG_TYPE_HANDLER(TYPE) \
template<>\
struct TypedArgReader<TYPE>\
{\
static bool Read(const std::string& value, void* pTargetVariable);\
};

    DEFINE_ARG_TYPE_HANDLER(bool);
    DEFINE_ARG_TYPE_HANDLER(int32_t);
    DEFINE_ARG_TYPE_HANDLER(float);
    DEFINE_ARG_TYPE_HANDLER(std::string);
    DEFINE_ARG_TYPE_HANDLER(std::vector<std::string>);
    DEFINE_ARG_TYPE_HANDLER(std::filesystem::path);

    class CommandLineManager
    {
    public:
        ~CommandLineManager() = default;
        CommandLineManager(const CommandLineManager&) = delete;
        CommandLineManager(CommandLineManager&&) = delete;
        CommandLineManager& operator=(const CommandLineManager&) = delete;
        CommandLineManager& operator=(CommandLineManager&&) = delete;

        static CommandLineManager& Get()
        {
            static CommandLineManager s_manager;
            return s_manager;
        }

        void RegisterArg(
            const std::string_view name,
            const std::string_view description,
            char shortSwitch,
            const std::string_view longSwitch,
            bool required,
            bool isFlag,
            void* targetVariable,
            bool(*readFunc)(const std::string&, void*))
        {
            m_Arguments.emplace_back(
                std::string{name},
                std::string{description},
                shortSwitch,
                std::string{longSwitch},
                required,
                false,
                isFlag,
                targetVariable,
                readFunc);
        }

        bool TryRead(int argc, char **argv);

    private:
        CommandLineManager() = default;

        struct Argument
        {
            std::string m_Name {};
            std::string m_Description {};
            char m_Switch {};
            std::string m_SwitchLong {};
            bool m_bIsRequired;
            bool m_bIsSet {};
            bool m_bIsFlag {};

            void* m_TargetVariable {nullptr};
            bool(*m_ReadFunc)(const std::string&, void*);
        };

        std::vector<Argument> m_Arguments;
    };

    // TODO: Add "IsSet" support
    template<typename T>
    class ArgRegistration final
    {
    public:
        ArgRegistration(
            const std::string_view& name,
            const std::string_view& description,
            const char shortSwitch,
            const std::string_view longSwitch,
            const bool isRequired,
            T defaultValue)
            : m_Value{defaultValue}
        {
            CommandLineManager::Get().RegisterArg(
                name,
                description,
                shortSwitch,
                longSwitch,
                isRequired,
                false,
                &m_Value,
                &TypedArgReader<T>::Read);
        }

        [[nodiscard]] const T& GetValue() const { return m_Value; }

    private:
        T m_Value;
    };

    template<>
    class ArgRegistration<bool> final
    {
    public:
        ArgRegistration(
            const std::string_view& name,
            const std::string_view& description,
            const char shortSwitch,
            const std::string_view longSwitch,
            const bool isRequired,
            const bool defaultValue)
            : m_Value{defaultValue}
        {
            CommandLineManager::Get().RegisterArg(
                name,
                description,
                shortSwitch,
                longSwitch,
                isRequired,
                true,
                &m_Value,
                &TypedArgReader<bool>::Read);
        }

        [[nodiscard]] const bool& GetValue() const { return m_Value; }

    private:
        bool m_Value{false};
    };

#define COMMANDLINECAT_NX(A, B) A ## B
#define COMMANDLINECAT(A, B) COMMANDLINECAT_NX(A, B)

#define DEFINE_COMMAND_ARG_SLEXT(NAME, TYPE, SWITCH_SHORT, SWITCH_LONG, DESCRIPTION, REQUIRED, DEFAULT)\
    namespace\
    {\
        namespace command_line::internal\
        {\
            atlas::core::command_line::ArgRegistration<TYPE> COMMANDLINECAT(g_arg, NAME){#NAME, DESCRIPTION, SWITCH_SHORT, SWITCH_LONG, REQUIRED, DEFAULT};\
        }\
        namespace command_line\
        {\
            const TYPE& COMMANDLINECAT(Get, NAME)() { return internal::COMMANDLINECAT(g_arg, NAME).GetValue(); };\
        }\
    }\

#define DEFINE_COMMAND_ARG_SL(NAME, TYPE, SWITCH_SHORT, DESCRIPTION) DEFINE_COMMAND_ARG_SLEXT(NAME, TYPE, SWITCH_SHORT, {}, DESCRIPTION, false, {})
#define DEFINE_COMMAND_ARG_S(NAME, TYPE, SWITCH_SHORT, DESCRIPTION) DEFINE_COMMAND_ARG_SLEXT(NAME, TYPE, SWITCH_SHORT, {}, DESCRIPTION, false, {})
#define DEFINE_COMMAND_ARG_L(NAME, TYPE, SWITCH_LONG, DESCRIPTION) DEFINE_COMMAND_ARG_SLEXT(NAME, TYPE, {}, SWITCH_LONG, DESCRIPTION, false, {})
#define DEFINE_REQUIRED_COMMAND_ARG_SL(NAME, TYPE, SWITCH_SHORT, DESCRIPTION) DEFINE_COMMAND_ARG_SLEXT(NAME, TYPE, SWITCH_SHORT, {}, DESCRIPTION, true, {})
#define DEFINE_REQUIRED_COMMAND_ARG_S(NAME, TYPE, SWITCH_SHORT, DESCRIPTION) DEFINE_COMMAND_ARG_SLEXT(NAME, TYPE, SWITCH_SHORT, {}, DESCRIPTION, true, {})
#define DEFINE_REQUIRED_COMMAND_ARG_L(NAME, TYPE, SWITCH_LONG, DESCRIPTION) DEFINE_COMMAND_ARG_SLEXT(NAME, TYPE, {}, SWITCH_LONG, DESCRIPTION, true, {})



}
