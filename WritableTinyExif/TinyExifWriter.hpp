//
//  TinyExifWriter.hpp
//  WritableTinyExif
//
//  Created by zhushiyu01 on 2020/11/30.
//

#ifndef TinyExifWriter_hpp
#define TinyExifWriter_hpp

#include <stdio.h>
#include <iostream> // std::cout
#include <fstream>  // std::ifstream
#include <vector>   // std::vector
#include "TinyEXIF.h"


// Jpeg图片格式说明：https://www.media.mit.edu/pia/Research/deepview/exif.html
// 这儿还有一个中文的介绍文章：https://www.jianshu.com/p/ae7b9ab20bca
//
// 关于Offset说明
// firstIfdOffset: TIFF Header中的 firstIfdOffset，其相对起点是TIFF Header的起始位置
// IFD0 内entry data中的数据Offset：起始位置也是TIFF Header起始位置
// subIFDOffset: 起始位置也是TIFF Header起始位置
// nextIFD-

namespace TinyEXIF {

#define EXIF_HEADER_START 0
#define TIFF_HEADER_START 10
#define TIFF_HEADER_LENGTH 8
#define TIFF_ENTRY_LENGTH 12

enum ExifEditCode {
    EDIT_SUCCESS           = 0, // 修改成功
    EDIT_CORRUPT_DATA      = 1 // 数据错误
};

class TINYEXIF_LIB ExifWriter {
public:
    /// 无参构造函数
    ExifWriter();
    /// Exif的原有数据
    ExifWriter(const uint8_t* originData, uint32_t len);
    /// 析构函数
    ~ExifWriter();
    
    /// 增加exif info信息
    /// @param info Exif信息
    bool addExifInfo(EXIFInfo *info);

    /// 读取一个文件，修改其exif后，输出到指定文件
    /// @param path 读取jpeg图片地址
    /// @param outputPath 输出的图片地址
    bool writeToFile (const char *path, const char *outputPath);
    
private:
    // 存储exif数据的buffer
    uint8_t *buffer;
    // exif数据的长度
    uint32_t bufferLen = 0;
    // 数据对齐方式，是大端还是小端。intel表示大端
    bool alignIntel = false;
    
    // 初始化原始数据
    void initOriginExifData();
    
    /// 添加属性
    /// @param tag  属性Tag
    /// @param value  属性值
    /// @param dataType  属性值类型，在JpegMarks.h中查找
    int addAttribute(int16_t tag, void *value, uint16_t dataType);
    
    /// 修改属性值
    /// @param tag  属性Tag
    /// @param value  属性值
    /// @param dataType 属性值类型，在JpegMarks.h中查找
    int editAttribute(int16_t tag, void *value, uint16_t dataType);
    
    /// 扩展Buffer，当插入属性或修改属性需要扩大exif区长度时，扩展buffer长度
    /// @param start  扩展起始index
    /// @param expandSize  扩展byte数量
    /// @param fillData  扩展时直接写入的数据，如果没有可以传NULL
    /// @param fillDataSize 写入数据长度
    void expandBuffer(uint32_t start, int32_t expandSize, uint8_t *fillData, uint32_t fillDataSize);
    
    /// 增加各属性的offset值，这个函数一般与expandBuffer配合使用，当扩展了buffer后，entry数据的offset等数据也要跟着改变
    /// @param offset 遍历entry的起始index
    /// @param startIndex  扩展数据的起始index，一般当前offeset大于此值时才需要修改offset
    /// @param increaseSize 增加数据长度
    void increaseTagOffsetData(uint32_t offset, uint32_t startIndex, int32_t increaseSize);
    
    /// 执行添加double类型的属性，它实际是addAttribute的扩展
    /// @param tag 属性Tag
    /// @param value 属性值
    /// @param dataType 属性类型，在JpegMarks.h中查找
    int executeAddDoubleAttribute(int16_t tag, double *value, uint16_t dataType);
    
    /// 执行添加string类型的属性，它实际是addAttribute的扩展
    /// @param tag 属性Tag
    /// @param value 属性值
    /// @param dataType 属性类型，在JpegMarks.h中查找
    int executeAddStringAttribute(int16_t tag, std::string *value, uint16_t dataType);
    
    /// 查询 某个属性的起始index
    /// @param offset 查找起始index
    /// @param tag 属性Tag
    uint32_t findTagOffset(uint32_t offset, int16_t tag);
    
    /// 查找数据区的起始index
    /// @param ifdDirectoryOffset IFD区的起始index
    uint32_t findDataAreaOffset(uint32_t ifdDirectoryOffset);
    
    /// 执行修改属性值
    /// @param offset 属性起始index
    /// @param value 属性值
    /// @param dataAreaOffset 数据区起始index
    void executeModifyAttributeValue(uint32_t offset, void *value, uint32_t dataAreaOffset);
    
    /// 执行修改整数类型属性的值，它实际是executeModifyAttributeValue的执行分支
    /// @param offset 属性起始Index
    /// @param value 属性值
    /// @param dataAreaOffset 数据区起始index
    void executeModifyAttributeIntValue(uint32_t offset, uint32_t *value, uint32_t dataAreaOffset);
    
    /// 执行修改小数类型属性的值，它实际是executeModifyAttributeValue的执行分支
    /// @param offset 属性起始Index
    /// @param value 属性值
    /// @param dataAreaOffset 数据区起始index
    void executeModifyAttributeDoubleValue(uint32_t offset, double *value, uint32_t dataAreaOffset);
    
    /// 执行修改字符串类型属性的值，它实际是executeModifyAttributeValue的执行分支
    /// @param offset 属性起始Index
    /// @param value 属性值
    /// @param dataAreaOffset 数据区起始index
    void executeModifyAttributeStringValue(uint32_t offset, std::string *value, uint32_t dataAreaOffset);
    
public:
    
    /// 读取一个Jpeg图片的exif数据
    /// @param imagePath jpeg图片地址
    /// @param len exif数据长度
    static uint8_t* readExifData(const char *imagePath, uint32_t &len);
    
    /// 计算一个数据的长度
    /// @param dataType 数据类型
    /// @param components component的数量
    static uint32_t computeDataSize(uint16_t dataType, uint32_t components);
    
};
}
#endif /* TinyExifWriter_hpp */
