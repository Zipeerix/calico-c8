#include <exception>
#include <string>
#include <ctime>
#include "Interpreter.hh"

Chip8Interpreter::Chip8Interpreter()
{
    for (auto i = 0; i < C8_FONTSET.size(); i++)
    {
        _memory[i + 0x050] = C8_FONTSET[i];
    }
}

void Chip8Interpreter::LoadROM(const std::vector<uint8_t>& binary)
{
    if (binary.size() > C8_MEMORY_SIZE - 0x200 || binary.empty())
    {
        throw std::invalid_argument("Invalid ROM file: Empty or too big for CHIP8");
    }

    for (auto i = 0; i < binary.size(); i++)
    {
        _memory[i + 0x200] = binary[i];
    }
}

Chip8FrameBuffer& Chip8Interpreter::AccessFrameBuffer()
{
    return _frame_buffer;
}

void Chip8Interpreter::HandleKeyEvent(CalicoEvent event, CalicoKey key)
{
    if (key == CalicoKey::Invalid || event == CalicoEvent::Invalid)
    {
        return;
    }

    int key_index = static_cast<int>(key);

    _keypad_status[key_index] = event == CalicoEvent::KeyDown;
}

void Chip8Interpreter::TickDelayTimer()
{
    if (_timers.delay > 0)
    {
        _timers.delay--;
    }
}

void Chip8Interpreter::TickSoundTimer()
{
    if (_timers.sound > 0)
    {
        _timers.sound--;
    }
}

bool Chip8Interpreter::ShouldPlaySound() const
{
    return _timers.sound != 0;
}

bool Chip8Interpreter::DrawFlag()
{
    return _draw_flag;
}

void Chip8Interpreter::DrawFlag(bool new_val)
{
    _draw_flag = new_val;
}

uint8_t Chip8Interpreter::GetXFromOpcode() const
{
    return (_current_opcode & 0x0F00) >> 8;
}

uint8_t Chip8Interpreter::GetYFromOpcode() const
{
    return (_current_opcode & 0x00F0) >> 4;
}

uint16_t Chip8Interpreter::GetNNNFromOpcode() const
{
    return _current_opcode & 0x0FFF;
}

uint8_t Chip8Interpreter::GetNNFromOpcode() const
{
    return _current_opcode & 0x00FF;
}

uint8_t Chip8Interpreter::GetNFromOpcode() const
{
    return _current_opcode & 0x000F;
}

void Chip8Interpreter::Draw(int x, int y, int height)
{
    const uint8_t x_cord = _registers.general[x];
    const uint8_t y_cord = _registers.general[y];

    bool pixel_flipped = false;

    for (auto diff_y = 0; diff_y < height; diff_y++)
    {
        uint8_t r = _memory[_registers.i + diff_y];

        for (auto diff_x = 0; diff_x < 8; diff_x++)
        {
            if ((r & (1 << (7 - diff_x))) != 0)
            {
                _frame_buffer.FlipPixel(x_cord + diff_x, y_cord + diff_y);
                if (!_frame_buffer.GetPixelFrom2DCords(x_cord + diff_x, y_cord + diff_y))
                {
                    pixel_flipped = true;
                }
            }
        }
    }

    _draw_flag = true;
    _registers.general[0xF] = pixel_flipped;
}

void Chip8Interpreter::FunctionCall(uint16_t address)
{
    _stack.push(_registers.pc);
    _registers.pc = address;
}

void Chip8Interpreter::FunctionReturn()
{
    // TODO is there exception for top when empty? If not then crash
    if (_stack.empty())
    {
        throw std::underflow_error("Call stack underflow at PC=" + std::to_string(_registers.pc - 2));
    }

    _registers.pc = _stack.top();
    _stack.pop();
}

void Chip8Interpreter::ExecuteNextInstruction()
{
    _current_opcode = (_memory[_registers.pc] << 8) | _memory[_registers.pc + 1];
    _registers.pc += 2;

    switch (_current_opcode & 0xF000)
    {
        case 0x0000:
            switch (_current_opcode)
            {
                case 0x00ee:
                    FunctionReturn();
                    break;

                case 0x00e0:
                    _frame_buffer.Clear();
                    _draw_flag = true;
                    break;


                default:
                    FunctionCall(GetNNNFromOpcode());
                    break;
            }
            break;

        case 0x1000:
            _registers.pc = GetNNNFromOpcode();
            break;

        case 0x2000:
            FunctionCall(GetNNNFromOpcode());
            break;

        case 0x3000:
            if (_registers.general[GetXFromOpcode()] == GetNNFromOpcode())
            {
                _registers.pc += 2;
            }
            break;

        case 0x4000:
            if (_registers.general[GetXFromOpcode()] != GetNNFromOpcode())
            {
                _registers.pc += 2;
            }
            break;

        case 0x5000:
            if (_registers.general[GetXFromOpcode()] == _registers.general[GetYFromOpcode()])
            {
                _registers.pc += 2;
            }
            break;

        case 0x6000:
            _registers.general[GetXFromOpcode()] = GetNNFromOpcode();
            break;

        case 0x7000:
            _registers.general[GetXFromOpcode()] = _registers.general[GetXFromOpcode()] + GetNNFromOpcode();
            break;

        case 0x8000:
            switch (_current_opcode & 0x000F)
            {
                case 0x0:
                    _registers.general[GetXFromOpcode()] = _registers.general[GetYFromOpcode()];
                    break;

                case 0x1:
                    _registers.general[GetXFromOpcode()] |= _registers.general[GetYFromOpcode()];
                    break;

                case 0x2:
                    _registers.general[GetXFromOpcode()] &= _registers.general[GetYFromOpcode()];
                    break;

                case 0x3:
                    _registers.general[GetXFromOpcode()] ^= _registers.general[GetYFromOpcode()];
                    break;

                case 0x4:
                {
                    uint16_t res = _registers.general[GetXFromOpcode()] + _registers.general[GetYFromOpcode()];

                    _registers.general[GetXFromOpcode()] = res;
                    _registers.general[0xF] = res > 0xFF;
                }
                    break;

                case 0x5:
                {
                    uint16_t res = _registers.general[GetXFromOpcode()] - _registers.general[GetYFromOpcode()];

                    _registers.general[GetXFromOpcode()] = res % 0x100;
                    _registers.general[0xF] = res >= 0;
                }
                    break;

                case 0x6:
                    _registers.general[0xF] = (_registers.general[GetXFromOpcode()] & 1) == 1;
                    _registers.general[GetXFromOpcode()] >>= 1;
                    break;

                case 0x7:
                {
                    uint16_t res = _registers.general[GetYFromOpcode()] - _registers.general[GetXFromOpcode()];

                    _registers.general[GetXFromOpcode()] = res % 0x100;
                    _registers.general[0xF] = res >= 0;
                }
                    break;

                case 0xE:
                    _registers.general[0xF] = (_registers.general[GetXFromOpcode()] & 0b10000000) == 0b10000000;
                    _registers.general[GetXFromOpcode()] <<= 1;
                    break;

                default:
                    throw std::runtime_error("Invalid opcode (" + std::to_string(_current_opcode) +
                                             ") at PC=(" + std::to_string(_registers.pc) + ")");

            }
            break;

        case 0x9000:
            if (_registers.general[GetXFromOpcode()] != _registers.general[GetYFromOpcode()])
            {
                _registers.pc += 2;
            }
            break;

        case 0xA000:
            _registers.i = GetNNNFromOpcode();
            break;

        case 0xB000:
            _registers.pc = GetNNNFromOpcode() + _registers.general[0];
            break;

        case 0xC000:
        {
            srand(static_cast <unsigned int> (time(nullptr)));

            _registers.general[GetXFromOpcode()] = (rand() % 0x100) & GetNNFromOpcode();
        }
            break;

        case 0xD000:
            Draw(GetXFromOpcode(), GetYFromOpcode(), GetNFromOpcode());
            break;

        case 0xE000:
            switch (_current_opcode & 0x00FF)
            {
                case 0x9E:
                    if (_keypad_status[_registers.general[GetXFromOpcode()]])
                    {
                        _registers.pc += 2;
                    }
                    break;

                case 0xA1:
                    if (!_keypad_status[_registers.general[GetXFromOpcode()]])
                    {
                        _registers.pc += 2;
                    }
                    break;

                default:
                    throw std::runtime_error("Invalid opcode (" + std::to_string(_current_opcode) +
                                             ") at PC=(" + std::to_string(_registers.pc) + ")");
            }
            break;

        case 0xF000:
            switch (_current_opcode & 0x00FF)
            {
                case 0x07:
                    _registers.general[GetXFromOpcode()] = _timers.delay;
                    break;

                case 0x0A:
                {
                    bool key_pressed = false;

                    for (auto i = 0; i < 16; i++)
                    {
                        if (_keypad_status[i])
                        {
                            _registers.general[GetXFromOpcode()] = i;
                            key_pressed = true;
                        }
                    }

                    // If not pressed, stay on this instruction
                    if (!key_pressed)
                    {
                        _registers.pc -= 2;
                    }
                }

                case 0x15:
                    _timers.delay = _registers.general[GetXFromOpcode()];
                    break;

                case 0x18:
                    _timers.sound = _registers.general[GetXFromOpcode()];

                case 0x1E:
                    _registers.i += _registers.general[GetXFromOpcode()];
                    break;

                case 0x29:
                    _registers.i = _registers.general[GetXFromOpcode()] * 5;
                    break;

                case 0x33:
                {
                    uint8_t reg_x = _registers.general[GetXFromOpcode()];

                    _memory[_registers.i] = reg_x / 100;
                    _memory[_registers.i + 1] = (reg_x / 10) % 10;
                    _memory[_registers.i + 2] = reg_x % 10;
                }
                    break;

                case 0x55:
                    for (auto i = 0; i <= GetXFromOpcode(); i++)
                    {
                        _memory[_registers.i + i] = _registers.general[i];
                    }
                    break;

                case 0x65:
                    for (auto i = 0; i <= GetXFromOpcode(); i++)
                    {
                        _registers.general[i] = _memory[_registers.i + i];
                    }
                    break;

                default:
                    throw std::runtime_error("Invalid opcode (" + std::to_string(_current_opcode) +
                                             ") at PC=(" + std::to_string(_registers.pc - 2) + ")");
            }
            break;

        default:
            throw std::runtime_error("Invalid opcode (" + std::to_string(_current_opcode) +
                                     ") at PC=(" + std::to_string(_registers.pc - 2) + ")");
    }
}
