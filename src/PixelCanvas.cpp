#include "PixelCanvas.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

PixelCanvas::PixelCanvas(int width, int height, int scale, const std::string& title)
    : width_(width),
      height_(height),
      scale_(scale),
      window_(nullptr),
      renderer_(nullptr),
      texture_(nullptr),
      pixels_(width * height, white())
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error(SDL_GetError());
    }

    window_ = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width_ * scale_,
        height_ * scale_,
        SDL_WINDOW_SHOWN
    );

    if (!window_) {
        throw std::runtime_error(SDL_GetError());
    }

    renderer_ = SDL_CreateRenderer(
        window_,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer_) {
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    }

    if (!renderer_) {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_RenderSetLogicalSize(renderer_, width_, height_);

    texture_ = SDL_CreateTexture(
        renderer_,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        width_,
        height_
    );

    if (!texture_) {
        throw std::runtime_error(SDL_GetError());
    }
}

PixelCanvas::~PixelCanvas() {
    if (texture_) {
        SDL_DestroyTexture(texture_);
    }

    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
    }

    if (window_) {
        SDL_DestroyWindow(window_);
    }

    SDL_Quit();
}

void PixelCanvas::clear(uint32_t color) {
    std::fill(pixels_.begin(), pixels_.end(), color);
}

void PixelCanvas::putPixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= width_) {
        return;
    }

    if (y < 0 || y >= height_) {
        return;
    }

    pixels_[y * width_ + x] = color;
}

void PixelCanvas::putNorm01(double nx, double ny, uint32_t color) {
    nx = std::clamp(nx, 0.0, 1.0);
    ny = std::clamp(ny, 0.0, 1.0);

    int x = static_cast<int>(std::round(nx * (width_ - 1)));

    // Invertimos Y porque en pantalla y=0 está arriba.
    int y = static_cast<int>(std::round((1.0 - ny) * (height_ - 1)));

    putPixel(x, y, color);
}

void PixelCanvas::putNormCentered(double x, double y, uint32_t color) {
    // Entrada esperada:
    // x = -1 izquierda, +1 derecha
    // y = -1 abajo,    +1 arriba

    double nx = (x + 1.0) * 0.5;
    double ny = (y + 1.0) * 0.5;

    putNorm01(nx, ny, color);
}

void PixelCanvas::render() {
    SDL_UpdateTexture(
        texture_,
        nullptr,
        pixels_.data(),
        width_ * sizeof(uint32_t)
    );

    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}

bool PixelCanvas::handleEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            return false;
        }
    }

    return true;
}

void PixelCanvas::delay(int milliseconds) {
    SDL_Delay(milliseconds);
}

int PixelCanvas::width() const {
    return width_;
}

int PixelCanvas::height() const {
    return height_;
}