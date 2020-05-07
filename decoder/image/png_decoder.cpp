//
//  png_decoder.cpp
//  DTLiving
//
//  Created by Dan Jiang on 2020/4/29.
//  Copyright © 2020 Dan Thought Studio. All rights reserved.
//

#include "png_decoder.h"

#include <iostream>
#include <fstream>

#include "png.h"

namespace dtliving {
namespace decoder {
namespace image {

typedef struct {
    const png_byte* data;
    const png_size_t size;
} DataHandle;

typedef struct {
    const DataHandle data;
    png_size_t offset;
} ReadDataHandle;

// 首先利用函数 png_get_io_ptr 取出设置在数据源函数中的指针，然后将 read_length 里面的数据按照要求的数量拷贝到 raw_data 这块内存区域中
static void read_png_data_callback(png_structp png_ptr, png_byte *raw_data, png_size_t read_length) {
    ReadDataHandle *handle = (ReadDataHandle *)png_get_io_ptr(png_ptr);
    const png_byte *png_src = handle->data.data + handle->offset;

    memcpy(raw_data, png_src, read_length);
    handle->offset += read_length;
}

PngDecoder::PngDecoder() {
}

PngDecoder::~PngDecoder() {
}

RawImageData PngDecoder::ReadImage(std::string file) {
    std::streampos size;
    std::ifstream file_handle(file, std::ios::binary | std::ios::ate);
    if (file_handle.is_open()) {
        size = file_handle.tellg();
        compressed_data_ = new char[(unsigned long)size];
        file_handle.seekg(0, std::ios::beg);
        file_handle.read(compressed_data_, (std::streamsize)size);
        file_handle.close();
    } else {
        exit(1);
    }
  
//    size_t actualSize = 0;
//    FILE *pngFile = fopen(file.c_str(), "rb");
//    if(NULL != pngFile){
//        fseek(pngFile, 0, SEEK_END);
//        int data_length = ftell(pngFile);
//        rewind(pngFile);
//        compressed_data_ = new char[data_length];
//        actualSize = fread(compressed_data_, 1, data_length, pngFile);
//    }

    // 调用上下文
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    // 图片信息
    png_infop info_ptr = png_create_info_struct(png_ptr);
    
    // 设置数据来源
    png_byte *png_data = (png_byte *)compressed_data_;
    png_size_t png_size = static_cast<png_size_t>(size);
    ReadDataHandle png_data_handle = (ReadDataHandle) {{ png_data, png_size }, 0};
    
    png_set_read_fn(png_ptr, &png_data_handle, read_png_data_callback);
    
    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cerr << "read " << file << " failed" << std::endl;
        exit(1);
    }
    
    // Xcode: Turn off settings in Compress PNG Files of Build Setings
    // 读取图片信息
    png_read_info(png_ptr, info_ptr);
        
    // 查询图片信息
    png_uint_32 width, height;
    int bit_depth, color_type;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
    
    // 这一步非常重要，开发者可以通过调用 png_set_xxx 函数指定输出数据的格式，比如 RGB888、ARGB8888 等输出数据格式
    // Convert transparency to full alpha
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
    }
    // Convert grayscale, if needed.
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }
    // Convert paletted images, if needed.
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
    }
    // Add alpha channel, if there is none (rationale: GL_RGBA is faster than GL_RGB on many GPUs)
    if (color_type == PNG_COLOR_TYPE_PALETTE || color_type == PNG_COLOR_TYPE_RGB) {
        png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);
    }
    // Ensure 8-bit packing
    if (bit_depth < 8) {
        png_set_packing(png_ptr);
    } else if (bit_depth == 16) {
        png_set_scale_16(png_ptr);
    }

    // 更新图片信息
    png_read_update_info(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
    
    // 读取解码后的数据
    png_size_t row_size = png_get_rowbytes(png_ptr, info_ptr);
    int data_length = static_cast<int>(row_size * height);
    png_byte *raw_image = new png_byte(data_length);
    png_byte *row_ptrs[height];
    for (png_uint_32 i = 0; i < height; i++) {
        row_ptrs[i] = raw_image + i * row_size;
    }
    png_read_image(png_ptr, &row_ptrs[0]);
    
    // 结束数据读取
    png_read_end(png_ptr, info_ptr);
    // 释放调用上下文
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    
    RawImageData image_data = { (int)width, (int)height, (void *)raw_image };
    return image_data;
}

void PngDecoder::DeleteImage() {
    delete[] compressed_data_;
}

}
}
}
