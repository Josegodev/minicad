#pragma once

#include <SDL.h>

#include <cstdint>
#include <string>
#include <vector>

class PixelCanvas {
public:
    PixelCanvas(int width, int height, int scale, const std::string& title);
    ~PixelCanvas();

    PixelCanvas(const PixelCanvas&) = delete;
    PixelCanvas& operator=(const PixelCanvas&) = delete;

    void clear(uint32_t color = white());

    void putPixel(int x, int y, uint32_t color = black());

    void putNorm01(double nx, double ny, uint32_t color = black());
    void putNormCentered(double x, double y, uint32_t color = black());

    void render();

    bool handleEvents();
    void delay(int milliseconds);

    int width() const;
    int height() const;

    static constexpr uint32_t white() {
        return 0xFFFFFFFF;
    }

    static constexpr uint32_t black() {
        return 0xFF000000;
    }

private:
    int width_;
    int height_;
    int scale_;

    SDL_Window* window_;
    SDL_Renderer* renderer_;
    SDL_Texture* texture_;

    std::vector<uint32_t> pixels_;
};