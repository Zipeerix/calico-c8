#include "FrameBuffer.hh"

Pixel Chip8FrameBuffer::GetPixelFrom2DCords(int x, int y) const
{
    uint32_t index = CalculateArrayIndexFrom2DCoordinates(x, y, CHIP8_RES_X, CHIP8_RES_Y) * sizeof(uint32_t);

    return _raw_framebuffer[index];
}

uint32_t* Chip8FrameBuffer::GetSDLPixelArray()
{
    return (uint32_t*) _raw_framebuffer.data();
}

void Chip8FrameBuffer::FlipPixel(int x, int y)
{
    uint32_t index = CalculateArrayIndexFrom2DCoordinates(x, y, CHIP8_RES_X, CHIP8_RES_Y) * sizeof(uint32_t);

    bool original_pixel_status = _raw_framebuffer[index] == 0xFF;

    _raw_framebuffer.at(index) = original_pixel_status ? 0x00 : 0xFF;
    _raw_framebuffer.at(index + 1) = original_pixel_status ? 0x00 : 0xFF;
    _raw_framebuffer.at(index + 2) = original_pixel_status ? 0x00 : 0xFF;
    _raw_framebuffer.at(index + 3) = original_pixel_status ? 0x00 : 0xFF;
}

void Chip8FrameBuffer::Clear()
{
    memset(_raw_framebuffer.data(), 0, CHIP8_RES_Y * CHIP8_RES_X * sizeof(uint32_t));
}
