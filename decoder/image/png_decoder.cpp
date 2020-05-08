//
//  png_decoder.cpp
//  DTLiving
//
//  Created by Dan Jiang on 2020/4/29.
//  Copyright © 2020 Dan Thought Studio. All rights reserved.
//
//  read libpng-1.6.37/libpng-manual.txt to know how to use libpng

#include "png_decoder.h"

#include <iostream>

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

PngDecoder::PngDecoder() {
}

PngDecoder::~PngDecoder() {
}

RawImageData PngDecoder::ReadImage(std::string file) {
    FILE *fp = fopen(file.c_str(), "rb");
    if (!fp) {
        std::cerr << "can not open file " << file << std::endl;
        exit(1);
    }

    // 判断文件是否是 png
    int number = 4;
    char header[number];
    if (fread(header, 1, number, fp) != number) {
        std::cerr << "can not read file header" << std::endl;
        exit(1);
    }
    int is_png = !png_sig_cmp(reinterpret_cast<png_const_bytep>(header), 0, number);
    if (!is_png) {
        std::cerr << "file is not png" << std::endl;
        exit(1);
    }

    // 调用上下文
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        std::cerr << "invoke png_create_read_struct failed" << std::endl;
        exit(1);
    }

    // 图片信息
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        std::cerr << "invoke png_create_info_struct failed" << std::endl;
        exit(1);
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cerr << "decode " << file << " failed" << std::endl;
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        exit(1);
    }

    // 设置数据来源
    png_init_io(png_ptr, fp);

    // 告诉 libpng 前面读取了几个字段来获取文件类型
    png_set_sig_bytes(png_ptr, number);

    // Xcode: Turn off settings in Compress PNG Files of Build Setings
    // 读取图片信息
    png_read_png(png_ptr, info_ptr, (PNG_TRANSFORM_SCALE_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_GRAY_TO_RGB), NULL);
        
    // 查询图片信息
    png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
    png_uint_32 height = png_get_image_height(png_ptr, info_ptr);

    // 获取每行的字节数量
    unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    // 解码后的像素内容的指针
    auto raw_image = new unsigned char[row_bytes * height];
    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
    // 填充到像素内容的指针上去
    for (int i = 0; i < height; ++i) {
        memcpy(raw_image + (row_bytes * i), row_pointers[i], row_bytes);
    }

    // 释放调用上下文
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    
    RawImageData image_data = { (int)width, (int)height, (void *)raw_image };
    return image_data;
}

}
}
}
