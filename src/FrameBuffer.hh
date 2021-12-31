#ifndef CALICOC8_FRAMEBUFFER_HH
#define CALICOC8_FRAMEBUFFER_HH

#include <cstdint>
#include <cstring>
#include <array>

typedef bool Pixel;

constexpr int CHIP8_RES_X = 64;
constexpr int CHIP8_RES_Y = 32;

constexpr int CalculateArrayIndexFrom2DCoordinates(int x, int y, int w, int h)
{
    return (y % h) * w + (x % w);
}

class Chip8FrameBuffer
{
public:
    Pixel GetPixelFrom2DCords(int x, int y) const;
    void FlipPixel(int x, int y);
    void Clear();

    uint32_t* GetSDLPixelArray();

private:
    std::array<uint8_t, 64 * 32 * sizeof(uint32_t)> _raw_framebuffer{0};
};

#endif //CALICOC8_FRAMEBUFFER_HH
