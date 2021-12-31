#include <iostream>
#include "CommandLine.hh"
#include "Emulator.hh"

int main(int argc, char** argv)
{
    if (argc < 2 || std::string(argv[1]) == "help")
    {
        std::cout << "usage: calico-c8 <rom-path or 'help'>" << std::endl;

        return -1;
    }

    ApplicationCmdSettings parsed_args{};
    std::string rom_path = argv[1];

    if (argc > 2)
    {
        std::vector<std::string> special_args(argv + 2, argv + argc);

        try
        {
            parsed_args = ParseSpecialArguments(special_args);
        }
        catch (const std::invalid_argument& e)
        {
            std::cout << e.what() << std::endl;

            return -2;
        }
    }

    Emulator emulator(parsed_args);

    return emulator.Run(rom_path);
}

