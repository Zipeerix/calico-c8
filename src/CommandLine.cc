#include <exception>
#include <vector>
#include <string>
#include "CommandLine.hh"

static std::vector<std::string> SplitStringByDelimeter(const std::string& str, char delimeter)
{
    std::vector<std::string> tokens;

    int start = 0;
    int end = 0;

    while ((start = str.find_first_not_of(delimeter, end)) != std::string::npos)
    {
        end = str.find(delimeter, start);

        tokens.push_back(str.substr(start, end - start));
    }

    return tokens;
}

ApplicationCmdSettings ParseSpecialArguments(const std::vector<std::string>& args)
{
    ApplicationCmdSettings application_cmd_settings{};

    for (auto& arg: args)
    {
        auto arg_tokens = SplitStringByDelimeter(arg, ':');

        if (arg_tokens[0] == "-no_sound")
        {
            if (arg_tokens.size() == 1)
            {
                application_cmd_settings.sound_enabled = false;
            }
            else
            {
                throw std::invalid_argument("Invalid command line argument: " + arg);
            }
        }
        else if (arg_tokens[0] == "-clock_speed")
        {
            if (arg_tokens.size() != 2)
            {
                throw std::invalid_argument("Invalid command line argument format: " + arg);
            }

            try
            {
                application_cmd_settings.clock_speed = std::stoi(arg_tokens[1]);
            }
            catch (const std::exception& e)
            {
                throw std::invalid_argument("Unable to parse value of command line argument: " + arg);
            }
        }
        else if (arg_tokens[0] == "-window_size")
        {
            if (arg_tokens.size() != 3)
            {
                throw std::invalid_argument("Invalid command line argument format: " + arg);
            }

            try
            {
                application_cmd_settings.window_size_x = std::stoi(arg_tokens[1]);
                application_cmd_settings.window_size_y = std::stoi(arg_tokens[2]);
            }
            catch (const std::exception& e)
            {
                throw std::invalid_argument("Unable to parse value of command line argument: " + arg);
            }
        }
        else
        {
            throw std::invalid_argument("Invalid command line argument: " + arg);
        }
    }

    return application_cmd_settings;
}
