#ifndef CALICOC8_EMULATOR_HH
#define CALICOC8_EMULATOR_HH

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <memory>
#include "CommandLine.hh"
#include "Interpreter.hh"

std::vector<uint8_t> ReadBinaryToVector(const std::string& path);

class Emulator
{
public:
    explicit Emulator(const ApplicationCmdSettings& args);

    int Run(const std::string& rom_path);

private:
    int InitSDL();
    void CleanupSDL();

    std::unique_ptr<Chip8Interpreter> _interpreter = std::make_unique<Chip8Interpreter>();

    ApplicationCmdSettings _args;

    bool _main_loop_running = true;

    SDL_Window* _window = nullptr;
    SDL_Renderer* _renderer = nullptr;
    SDL_Texture* _frame_buffer_texture = nullptr;
    SDL_Event _event{};
    SDL_AudioSpec _audio_spec{};
    std::string _sdl_error_message;
    int _audio_sample_number = 0;
};

#endif //CALICOC8_EMULATOR_HH
