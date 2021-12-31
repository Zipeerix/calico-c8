#ifndef CALICOC8_COMMANDLINE_HH
#define CALICOC8_COMMANDLINE_HH

struct ApplicationCmdSettings
{
    bool sound_enabled = true;
    int window_size_x = 640;
    int window_size_y = 320;
    uint32_t clock_speed = 600;
};

ApplicationCmdSettings ParseSpecialArguments(const std::vector<std::string>& args);

#endif //CALICOC8_COMMANDLINE_HH
