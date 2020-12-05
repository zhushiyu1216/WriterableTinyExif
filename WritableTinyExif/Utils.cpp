//
//  Utils.cpp
//  WritableTinyExif
//
//  Created by zhushiyu01 on 2020/12/5.
//

#include <stdio.h>
#include <iostream> // std::cout

#include "Utils.h"

namespace Utils {

template< typename T >
std::string int_to_hex(T w, size_t hex_len) {
    static const char* digits = "0123456789ABCDEF";
    std::string rc(hex_len + 2,'0');
    rc[0] = '0';
    rc[1] = 'x';
    for (size_t i=0, j=(hex_len-1)*4 ; i<hex_len; ++i,j-=4) {
        rc[i + 2] = digits[(w>>j) & 0x0f];
    }
    
    return rc;
}

uint8_t parse8(const uint8_t* buf) {
    return buf[0];
}

uint16_t parse16(const uint8_t* buf, bool intel) {
    if (intel)
        return ((uint16_t)buf[1]<<8) | buf[0];
    return ((uint16_t)buf[0]<<8) | buf[1];
}

uint32_t parse32(const uint8_t* buf, bool intel) {
    if (intel)
        return ((uint32_t)buf[3]<<24) |
            ((uint32_t)buf[2]<<16) |
            ((uint32_t)buf[1]<<8)  |
            buf[0];
    return ((uint32_t)buf[0]<<24) |
        ((uint32_t)buf[1]<<16) |
        ((uint32_t)buf[2]<<8)  |
        buf[3];
}

float parseFloat(const uint8_t* buf, bool intel) {
    union {
        uint32_t i;
        float f;
    } i2f;
    i2f.i = parse32(buf, intel);
    return i2f.f;
}

double parseRational(const uint8_t* buf, bool intel, bool isSigned) {
    const uint32_t denominator = parse32(buf+4, intel);
    if (denominator == 0)
        return 0.0;
    const uint32_t numerator = parse32(buf, intel);
    return isSigned ?
        (double)(int32_t)numerator/(double)(int32_t)denominator :
        (double)numerator/(double)denominator;
}

std::string parseString(const uint8_t* buf,
    unsigned num_components,
    unsigned data,
    unsigned base,
    unsigned len,
    bool intel) {
    std::string value;
    if (num_components <= 4) {
        value.resize(num_components);
        char j = intel ? 0 : 24;
        char j_m = intel ? -8 : 8;
        for (unsigned i=0; i<num_components; ++i, j -= j_m)
            value[i] = (data >> j) & 0xff;
        if (value[num_components-1] == '\0')
            value.resize(num_components-1);
    } else
    if (base+data+num_components <= len) {
        const char* const sz((const char*)buf+base+data);
        unsigned num(0);
        while (num < num_components && sz[num] != '\0')
            ++num;
        while (num && sz[num-1] == ' ')
            --num;
        value.assign(sz, num);
    }
    return value;
}

void convertInt32ToByteArray(uint32_t value, uint8_t *result, bool intel) {
    if (intel) {
        result[3] = (uint8_t)(value >> 24);
        result[2] = (uint8_t)((value & 0xFF0000) >> 16);
        result[1] = (uint8_t)((value & 0xFF00) >> 8);
        result[0] = (uint8_t)(value & 0xFF);
    } else {
        result[0] = (uint8_t)(value >> 24);
        result[1] = (uint8_t)((value & 0xFF0000) >> 16);
        result[2] = (uint8_t)((value & 0xFF00) >> 8);
        result[3] = (uint8_t)(value & 0xFF);
    }
}

void convertInt16ToByteArray(uint16_t value, uint8_t *result, bool intel) {
    if (intel) {
        result[1] = (uint8_t)((value & 0xFF00) >> 8);
        result[0] = (uint8_t)(value & 0xFF);
    } else {
        result[0] = (uint8_t)((value & 0xFF00) >> 8);
        result[1] = (uint8_t)(value & 0xFF);
    }
}

void convertInt8ToByteArray(uint8_t value, uint8_t *result, bool intel) {
    result[0] = (uint8_t)(value & 0xFF);
}

void printByteArrayByHex(uint8_t *data, uint32_t len) {
    uint32_t index = 0;
    while (index < len) {
        if (index % 12 == 0) {
            std::cout << std::endl;
        }
        for (int i=0; i<12 && index < len; i++) {
            if (index % 2 == 0) {
                std::cout << "  ";
            }
            std::cout << int_to_hex(data[index++]);
        }
    }
    
    std::cout << std::endl << std::endl;
}

void convertDoubleToFraction(double data, int &numerator, int &denominator) {

    for (int j = 10;  ;j *= 10) {
        double fractional_part = data * j - floor(data * j);
        if (fractional_part < 0.00001 || j > 100000) {
            numerator = (int)(data  * j);
            denominator = j;
            break;
        }
    }
    
    if (denominator == 0 || numerator == 0) {
        numerator = 0;
        denominator = 0;
        return;
    }
    
    // 简化分数
    int min = numerator;
    int max = denominator;
    while (true) {
        int mod = max % min;
        if (mod == 0) {
            break;
        }
        
        max = min;
        min = mod;
    }
    numerator /= min;
    denominator /= min;
}
}
