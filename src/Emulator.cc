#include <exception>
#include <iostream>
#include <fstream>
#include <string>
#include "Emulator.hh"

std::vector<uint8_t> ReadBinaryToVector(const std::string &path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.good()) {
        throw std::invalid_argument("Invalid ROM path: " + path);
    }

    file.unsetf(std::ios::skipws);

    file.seekg(0, std::ios::end);
    std::streampos file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> vec;
    vec.reserve(file_size);

    vec.insert(vec.begin(),
               std::istream_iterator<uint8_t>(file),
               std::istream_iterator<uint8_t>());

    return vec;
}

Emulator::Emulator(const ParsedArgs &args) {
    _args = args;
}

// Used to keep interpreter as separate module from SDL
static CalicoEvent TranslateSDLEventToCalicoEvent(uint32_t sdl_event_type) {
    switch (sdl_event_type) {
        case SDL_KEYDOWN:
            return CalicoEvent::KeyDown;

        case SDL_KEYUP:
            return CalicoEvent::KeyUp;

        default:
            return CalicoEvent::Invalid;
    }
}

// Used to keep interpreter as separate module from SDL
static CalicoKey TranslateSDLKeyToCalicoKey(SDL_Keycode sdl_key) {
    switch (sdl_key) {
        case SDLK_1:
            return CalicoKey::MK1;

        case SDLK_2:
            return CalicoKey::MK2;

        case SDLK_3:
            return CalicoKey::MK3;

        case SDLK_4:
            return CalicoKey::MK4;

        case SDLK_q:
            return CalicoKey::Q;

        case SDLK_w:
            return CalicoKey::W;

        case SDLK_e:
            return CalicoKey::E;

        case SDLK_r:
            return CalicoKey::R;

        case SDLK_a:
            return CalicoKey::A;

        case SDLK_s:
            return CalicoKey::S;

        case SDLK_d:
            return CalicoKey::D;

        case SDLK_f:
            return CalicoKey::F;

        case SDLK_z:
            return CalicoKey::Z;

        case SDLK_x:
            return CalicoKey::X;

        case SDLK_c:
            return CalicoKey::C;

        case SDLK_v:
            return CalicoKey::V;

        default:
            return CalicoKey::Invalid;
    }
}

static void SDLAudioCallBack(void *user_data, Uint8 *raw_buffer, int bytes) {
    auto *buffer = (Sint16 *) raw_buffer;
    int length = bytes / 2;
    int &sample_nr = *((int *) user_data);

    for (auto i = 0; i < length; i++, sample_nr++) {
        double time = (double) sample_nr / (double) 44100;
        buffer[i] = (Sint16) (28000 * sin(2.0f * M_PI * 441.0f * time));
    }
}

int Emulator::InitSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        _sdl_error_message = SDL_GetError();
        return -1;
    }

    SDL_AudioSpec audio_spec_request;
    audio_spec_request.freq = 44100;
    audio_spec_request.format = AUDIO_S16SYS;
    audio_spec_request.channels = 1;
    audio_spec_request.samples = 2048;
    audio_spec_request.callback = SDLAudioCallBack;
    audio_spec_request.userdata = &_audio_sample_number;

    if (SDL_OpenAudio(&audio_spec_request, &_audio_spec) != 0) {
        _sdl_error_message = SDL_GetError();
        return -2;
    }

    _window = SDL_CreateWindow("CalicoC8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               _args.window_size_x, _args.window_size_y, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (_window == nullptr) {
        _sdl_error_message = "Unable to create SDL Window";
        return -2;
    }

    _renderer = SDL_CreateRenderer(_window, 0, SDL_RENDERER_ACCELERATED);
    if (_renderer == nullptr) {
        _sdl_error_message = "Unable to create SDL Renderer";
        return -3;
    }

    _frame_buffer_texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ABGR8888,
                                              SDL_TEXTUREACCESS_STATIC, 64, 32);
    if (_frame_buffer_texture == nullptr) {
        _sdl_error_message = "Unable to create SDL Texture for frame buffer";
        return -4;
    }

    return 0;
}

void Emulator::CleanupSDL() {
    SDL_DestroyTexture(_frame_buffer_texture);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
    SDL_Quit();
}

int Emulator::Run(const std::string &rom_path) {
    try {
        _interpreter->LoadROM(ReadBinaryToVector(rom_path));
    } catch (const std::invalid_argument &e) {
        std::cout << e.what() << std::endl;

        return -1;
    }


    int init_sdl_res = InitSDL();
    if (init_sdl_res != 0) {
        CleanupSDL();

        std::cout << "Error: " << _sdl_error_message << std::endl;

        return init_sdl_res;
    }

    while (_main_loop_running) {
        uint64_t start = SDL_GetPerformanceCounter();

        while (SDL_PollEvent(&_event) != 0) {
            if (_event.type == SDL_QUIT) {
                _main_loop_running = false;
            } else if (_event.type == SDL_KEYDOWN || _event.type == SDL_KEYUP) {
                _interpreter->HandleKeyEvent(TranslateSDLEventToCalicoEvent(_event.type),
                                             TranslateSDLKeyToCalicoKey(_event.key.keysym.sym));
            }
        }

        // Ex. 600 hz clock speed (600hz / 60fps = 10 instructions per second)
        for (auto i = 0; i < _args.clock_speed / 60; i++) {
            try {
                _interpreter->ExecuteNextInstruction();
            } catch (const std::exception &e) {
                std::cout << e.what();

                return -2;
            }
        }

        _interpreter->TickSoundTimer();
        _interpreter->TickDelayTimer();

        if (_interpreter->ShouldPlaySound() && _args.sound_enabled) {
            SDL_PauseAudio(0);
            SDL_Delay(10); // TODO maybe find a way without delay?
            SDL_PauseAudio(1);
        }

        if (_interpreter->IsDrawFlagSet()) {
            SDL_UpdateTexture(_frame_buffer_texture, nullptr, _interpreter->AccessFrameBuffer().GetSDLPixelArray(),
                              64 * sizeof(uint32_t));

            SDL_RenderClear(_renderer);
            SDL_RenderCopy(_renderer, _frame_buffer_texture, nullptr, nullptr);
            SDL_RenderPresent(_renderer);

            _interpreter->DisableDrawFlag();
        }

        uint64_t end = SDL_GetPerformanceCounter();

        float elapsedMS = (float) (end - start) / (float) SDL_GetPerformanceFrequency() * 1000.0f;

        // FPS cap set to 60
        SDL_Delay(floor(16.666f - elapsedMS));
    }

    CleanupSDL();

    return 0;
}