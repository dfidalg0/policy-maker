#ifndef __UTILS_CMD_PARSE_HH__
#define __UTILS_CMD_PARSE_HH__

#include <string>
#include <unordered_map>
#include <vector>

namespace utils {
    enum class ArgType {
        ABSCENT,
        NONE,
        STRING,
        INT,
        FLOAT,
    };

    class Arg {
    public:
        Arg(ArgType type = ArgType::ABSCENT, char const * value = "");
        Arg(char const * value);
        Arg(int value);
        Arg(double value);

        inline ArgType type() const { return _type; }

        operator std::string() const;
        operator int() const;
        operator double() const;
        operator bool() const;
    private:
        ArgType _type;

        union {
            char const * _string;
            int _int;
            double _float;
        } _value;
    };

    class CmdParser {
    public:
        struct ArgInfo {
            std::string name;
            ArgType type;
            bool required;
            bool positional;
            std::string description;
            std::string shorthand = "";
            Arg default_value = Arg();
        };

        typedef std::vector<ArgInfo> PositionalArgSpec;
        typedef std::unordered_map<std::string, ArgInfo> NamedArgSpec;

        struct ArgSpec {
            PositionalArgSpec positional;
            NamedArgSpec named;
            NamedArgSpec shorthands;
            std::pair<std::string, std::string> help;
        };

        class Result {
        public:
            Result(int argc, char const * argv[], const ArgSpec& spec);

            Arg get(std::string name);
            bool has(std::string name);

            Arg operator[](std::string name);
            inline const std::vector<std::string>& rest() const { return _rest; }
            inline const bool is_help() const { return _is_help; }
        private:
            std::unordered_map<std::string, Arg> _args;
            std::vector<std::string> _rest;
            bool _is_help;
        };

        CmdParser();

        CmdParser & add_arg(const ArgInfo& info);
        CmdParser & add_help_arg(const std::string& name = "help", const std::string& shorthand = "h");
        CmdParser & set_rest_description(const std::string& description);

        Result parse(int argc, char const * argv[]) const;

        std::string help(std::string name) const;
    private:
        ArgSpec _spec;
        std::string _rest_description;

        void _add_to_map(std::string name, const ArgInfo& info, bool shorthand = false);
    };
};

#endif // __UTILS_CMD_PARSE_HH__
