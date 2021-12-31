#ifndef CALICOC8_INTERPRETER_HH
#define CALICOC8_INTERPRETER_HH

#include <cstdint>
#include <vector>
#include <array>
#include <stack>
#include "FrameBuffer.hh"

constexpr int C8_MEMORY_SIZE = 4096;

static constexpr std::array<uint8_t, 80> C8_FONTSET
        {
                0xF0, 0x90, 0x90, 0x90, 0xF0,
                0x20, 0x60, 0x20, 0x20, 0x70,
                0xF0, 0x10, 0xF0, 0x80, 0xF0,
                0xF0, 0x10, 0xF0, 0x10, 0xF0,
                0x90, 0x90, 0xF0, 0x10, 0x10,
                0xF0, 0x80, 0xF0, 0x10, 0xF0,
                0xF0, 0x80, 0xF0, 0x90, 0xF0,
                0xF0, 0x10, 0x20, 0x40, 0x40,
                0xF0, 0x90, 0xF0, 0x90, 0xF0,
                0xF0, 0x90, 0xF0, 0x10, 0xF0,
                0xF0, 0x90, 0xF0, 0x90, 0x90,
                0xE0, 0x90, 0xE0, 0x90, 0xE0,
                0xF0, 0x80, 0x80, 0x80, 0xF0,
                0xE0, 0x90, 0x90, 0x90, 0xE0,
                0xF0, 0x80, 0xF0, 0x80, 0xF0,
                0xF0, 0x80, 0xF0, 0x80, 0x80,
        };

enum class CalicoKey
{
    // MK = MainKeyboard
    MK1,
    MK2,
    MK3,
    MK4,
    Q,
    W,
    E,
    R,
    A,
    S,
    D,
    F,
    Z,
    X,
    C,
    V,
    Invalid
};

enum class CalicoEvent
{
    KeyUp,
    KeyDown,
    Invalid
};

class Chip8Interpreter
{
public:
    Chip8Interpreter();

    void LoadROM(const std::vector<uint8_t>& binary);
    void HandleKeyEvent(CalicoEvent event, CalicoKey key);

    void TickDelayTimer();
    void TickSoundTimer();
    bool ShouldPlaySound() const;

    bool DrawFlag();
    void DrawFlag(bool new_val);

    uint8_t GetXFromOpcode() const;
    uint8_t GetYFromOpcode() const;
    uint16_t GetNNNFromOpcode() const;
    uint8_t GetNNFromOpcode() const;
    uint8_t GetNFromOpcode() const;

    Chip8FrameBuffer& AccessFrameBuffer();

    void Draw(int x, int y, int height);
    void FunctionCall(uint16_t address);
    void FunctionReturn();

    void ExecuteNextInstruction();

private:
    Chip8FrameBuffer _frame_buffer{};
    std::array<uint8_t, C8_MEMORY_SIZE> _memory{0};
    std::stack<uint16_t> _stack;
    std::array<bool, 16> _keypad_status{0};

    uint16_t _current_opcode = 0x0000;

    bool _draw_flag = false;

    struct
    {
        std::array<uint8_t, 16> general{0};
        uint16_t pc = 0x200;
        uint16_t i = 0x00;
    } _registers;

    struct
    {
        uint8_t delay = 0x00;
        uint8_t sound = 0x00;
    } _timers;
};

#endif //CALICOC8_INTERPRETER_HH
