/// Copyright (c) 2020 Vitaly Chipounov
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.

#include <libpng/png.h>

#include "Image.h"
#include "Utils.h"

namespace {
struct DataPtr {
    uint8_t *data;
    size_t offset;
    size_t size;
};

void PngRead(png_structp pngPtr, png_bytep data, png_size_t length) {
    auto a = reinterpret_cast<DataPtr *>(png_get_io_ptr(pngPtr));

    if (a->offset + length >= a->size) {
        length = a->size - a->offset;
    }

    memcpy(data, &a->data[a->offset], length);
    a->offset += length;
}

} // namespace

namespace gui {

CFrameBufferPtr LoadImage(const std::string &path) {
    DataPtr ptr;
    png_structp png = nullptr;
    png_infop info = nullptr;
    png_uint_32 width, height, bitdepth, channels, colorType;
    std::unique_ptr<png_bytep[]> rows;
    CFrameBufferPtr fb;
    int stride;
    uint32_t *data;

    size_t size;
    auto file = utils::ReadFile(path, size);
    if (!file) {
        goto err;
    }

    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        goto err;
    }

    info = png_create_info_struct(png);
    if (!info) {
        goto err;
    }

    if (setjmp(png_jmpbuf(png))) {
        goto err;
    }

    ptr.data = file.get();
    ptr.offset = 0;
    ptr.size = size;
    png_set_read_fn(png, &ptr, PngRead);

    png_read_info(png, info);

    width = png_get_image_width(png, info);
    height = png_get_image_height(png, info);
    bitdepth = png_get_bit_depth(png, info);
    channels = png_get_channels(png, info);
    colorType = png_get_color_type(png, info);

    switch (colorType) {
        case PNG_COLOR_TYPE_PALETTE:
            png_set_palette_to_rgb(png);
            channels = 3;
            break;
        case PNG_COLOR_TYPE_GRAY:
            if (bitdepth < 8) {
                png_set_expand_gray_1_2_4_to_8(png);
            }
            bitdepth = 8;
            break;
    }

    // Convert transparency to alpha channel
    if (png_get_valid(png, info, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png);
        channels += 1;
    }

    // Round down 16 bits to 8
    if (bitdepth == 16) {
        png_set_strip_16(png);
    }

    assert(bitdepth == 8 && channels == 4);

    png_read_update_info(png, info);

    stride = width * bitdepth * channels / 8;

    fb = CFrameBuffer::Create(nullptr, width, height, stride);
    data = fb->Pixels();
    rows = std::unique_ptr<png_bytep[]>(new png_bytep[height]);

    for (size_t i = 0; i < height; i++) {
        png_uint_32 q = i * stride;
        rows[i] = (png_bytep) data + q;
    }

    png_read_image(png, rows.get());

err:
    if (png || info) {
        png_destroy_read_struct(&png, &info, nullptr);
    }
    return fb;
}

} // namespace gui
