//
//  Utils.h
//  WritableTinyExif
//
//  Created by zhushiyu01 on 2020/11/29.
//

#ifndef Utils_h
#define Utils_h

#include <iostream> // std::cout

namespace Utils {
    
/// 将十进制转换为十六进制，并生成字符串
/// @param w  十进制
template< typename T > std::string int_to_hex(T w, size_t hex_len = sizeof(T) << 1);

/// 解析byte
/// @param buf byte数组
uint8_t parse8(const uint8_t* buf);


/// 解析short
/// @param buf byte数组
/// @param intel 对齐方式，大端还是小端。intel使用的是大端
uint16_t parse16(const uint8_t* buf, bool intel);

/// 解析int
/// @param buf byte数组
/// @param intel 对齐方式，大端还是小端。intel使用的是大端
uint32_t parse32(const uint8_t* buf, bool intel);

/// 解析float
/// @param buf byte数组
/// @param intel 对齐方式，大端还是小端。intel使用的是大端
float parseFloat(const uint8_t* buf, bool intel);

/// 解析double
/// @param buf byte数组
/// @param intel 对齐方式，大端还是小端。intel使用的是大端
double parseRational(const uint8_t* buf, bool intel, bool isSigned);
std::string parseString(const uint8_t* buf,
    unsigned num_components,
    unsigned data,
    unsigned base,
    unsigned len,
    bool intel);

/// 将int转转换为byte数组
/// @param value  int
/// @param result 要存储到的byte数组
/// @param intel 对齐方式，大端还是小端。intel使用的是大端
void convertInt32ToByteArray(uint32_t value, uint8_t *result, bool intel);

/// 将short转转换为byte数组
/// @param value  int
/// @param result 要存储到的byte数组
/// @param intel 对齐方式，大端还是小端。intel使用的是大端
void convertInt16ToByteArray(uint16_t value, uint8_t *result, bool intel);

/// 将byte转转换为byte数组
/// @param value  int
/// @param result 要存储到的byte数组
/// @param intel 对齐方式，大端还是小端。intel使用的是大端
void convertInt8ToByteArray(uint8_t value, uint8_t *result, bool intel);


/// 将double转换为分数
/// @param data  double
/// @param numerator 分子
/// @param denominator 分母
void convertDoubleToFraction(double data, int &numerator, int &denominator);

/// 将byte[]使用十六进制打印
/// @param data byte[] byte数组指针
/// @param len 数组长度
void printByteArrayByHex(uint8_t *data, uint32_t len);

}
#endif /* Utils_h */
