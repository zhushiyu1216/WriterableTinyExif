//
//  TinyExifWriter.cpp
//  WritableTinyExif
//
//  Created by zhushiyu01 on 2020/11/30.
//

#include "TinyExifWriter.hpp"
#include "JpegMarks.h"
#include "Utils.h"

#include <iostream>

namespace TinyEXIF {

// Constructors
ExifWriter::ExifWriter() {
    initOriginExifData();
}

ExifWriter::ExifWriter(const uint8_t* originData, uint32_t len) {
    bufferLen = len;
    buffer = new uint8_t[bufferLen];
    memcpy(buffer, originData, bufferLen * sizeof(uint8_t)); //内存拷贝
    
    alignIntel = len >= 12 && originData[10] == 'I' && originData[11] == 'I';
}

ExifWriter::~ExifWriter() {
    delete [] buffer;
}

// 初始化原始数据，原始数据的exif内没有任何信息
void ExifWriter::initOriginExifData() {
    bufferLen = 24;
    buffer = new uint8_t[bufferLen];; //空数据也需要22位来标识
    uint32_t offset = 0;
    
    // APP1 标志
    uint8_t app1[] = {0xFF, 0xE1};
    memcpy(buffer + offset, app1, 2 * sizeof(uint8_t)); //内存拷贝
    offset += 2;
    
    // 总长度
    uint8_t len[] = {0x00, (uint8_t)(bufferLen - 2)};
    memcpy(buffer + offset, len, 2 * sizeof(uint8_t)); //内存拷贝
    offset += 2;
    
    // Exif Header
    char const *exif = "Exif\0\0";
    memcpy(buffer + offset, exif, 6 * sizeof(uint8_t)); //内存拷贝
    offset += 6;
    
    // 以下是TIFF Header部分----------------------------TIFF header begin
    char const *intelFlag = "MM";
    memcpy(buffer + offset, intelFlag, 2 * sizeof(uint8_t)); //内存拷贝
    offset += 2;
    
    uint8_t tagMark[] = {0x00, 0x2a};
    memcpy(buffer + offset, tagMark, 2 * sizeof(uint8_t)); //内存拷贝
    offset += 2;
    
    uint8_t firstIfdOffset[] = {0x00, 0x00, 0x00, 0x08};
    memcpy(buffer + offset, firstIfdOffset, 4 * sizeof(uint8_t)); //内存拷贝
    offset += 4;
    // TIFF header结束-------------------------------TIFF header end
    
    // Entry数量
    uint8_t entryNumber[] = {0x00, 0x00};
    memcpy(buffer + offset, entryNumber, 2 * sizeof(uint8_t)); //内存拷贝
    offset += 2;
    
    // 下一个IFD的地址
    uint8_t nextIFD[] = {0x00, 0x00, 0x00, 0x00};
    memcpy(buffer + offset, nextIFD, 4 * sizeof(uint8_t)); //内存拷贝
    offset += 4;
    
    if (offset != bufferLen) {
        throw "初始buffer数量不对!";
    }
}

    // 增加exif info信息
bool ExifWriter::addExifInfo(EXIFInfo *info) {
    if (info->ImageWidth) { //宽
        editAttribute(IMAGE_WIDTH, &(info->ImageWidth), TYPE_UINT32);
    }
    if (info->ImageHeight) { //高
        editAttribute(IMAGE_HEIGHT, &(info->ImageHeight), TYPE_UINT32);
    }
    if (!info->Software.empty()) { // software
        editAttribute(IMAGE_SOFTWARE, &(info->Software), TYPE_STRING);
    }
    if (!info->DateTimeOriginal.empty()) {
        editAttribute(DATETIME_ORIGINAL, &(info->DateTimeOriginal), TYPE_STRING);
    }
    if (info->ExposureProgram) {
        editAttribute(EXPOSURE_PROGRAM, &(info->ExposureProgram), TYPE_UINT16);
    }
    if (!info->GeoLocation.GPSDateStamp.empty()) {
        editAttribute(GEO_DATE_STAMP, &(info->GeoLocation.GPSDateStamp), TYPE_STRING);
    }
    if (info->FNumber) {
        editAttribute(IFD_FNUMBER, &(info->FNumber), TYPE_URATIONAL);
    }
    if (info->ApertureValue) {
        double value = info->ApertureValue;
        value = log(value) / 0.5 / log(2);
        editAttribute(IFD_APERTUREVALUE, &value, TYPE_URATIONAL);
    }
    
    return true;
}

    // 写入文件
    // 返回写入后的文件
bool ExifWriter::writeToFile (const char *path, const char *outputPath) {
    // 源文件读取流
    std::ifstream in (path, std::ios::in | std::ios::binary);
    if (!in.is_open()) {
        return false;
    }
    
    uint8_t tempBuf[512];
    
    //获取文件长度
    in.seekg(0, std::ios_base::end);
    uint64_t fileSize = in.tellg();
    uint64_t start = 0;
    in.seekg(start);
    
    // 跳转到APP0结尾
    in.read((char*)tempBuf, 6);
    uint16_t exifLen = Utils::parse16(tempBuf + 4, false);
    start = exifLen + 4;
    in.seekg(start);
    
    //写文件流
    std::ofstream out (outputPath, std::ios::out | std::ios::binary);
    if (!out.is_open()) {
        in.close();
        return false;
    }
    out.write((char*)tempBuf, 2); //jpeg头
    out.write((char*)buffer, bufferLen);
    while (start < fileSize) {
        uint64_t size = 512;
        uint64_t remain = fileSize - start;
        if (size > remain) {
            size = remain;
        }
        in.read((char *)tempBuf, size);
        out.write((char *)tempBuf, size);
        
        start += size;
    }
    
    in.close();
    out.close();
    return true;
}

//  修改一个Tag Entry的数据
int ExifWriter::editAttribute(int16_t tag, void *value, uint16_t dataType) {
    unsigned offset = (TIFF_HEADER_START + TIFF_HEADER_LENGTH - 4);
    const unsigned first_ifd_offset = Utils::parse32(buffer + offset, alignIntel);
    offset += first_ifd_offset - 4; //数据起始于MM+002a
    if (offset + 2 > bufferLen) {
        return EDIT_CORRUPT_DATA;
    }
    
    bool isFound = false;
    
    uint32_t tagEntryOffset = findTagOffset(offset, tag);
    if (tagEntryOffset > 0) { //找到tag
        executeModifyAttributeValue(tagEntryOffset, value, findDataAreaOffset(offset));
        isFound = true;
    } else {
        uint32_t subIFDOffset = findTagOffset(offset, SUB_IFD_OFFSET);
        if (subIFDOffset > 0) { // 在Sub IFD中再查找
            subIFDOffset = Utils::parse32(buffer + subIFDOffset + 8, alignIntel);
            tagEntryOffset = findTagOffset(subIFDOffset + TIFF_HEADER_START, tag);
            if (tagEntryOffset > 0) {
                executeModifyAttributeValue(tagEntryOffset, value, findDataAreaOffset(subIFDOffset + TIFF_HEADER_START));
                isFound = true;
            }
        }
        
        uint32_t gpsIFDOffset = findTagOffset(offset, GPS_IFD_OFFSET);
        if (gpsIFDOffset > 0) { // 在GPS IFD中再查找
            gpsIFDOffset = Utils::parse32(buffer + gpsIFDOffset + 8, alignIntel);
            tagEntryOffset = findTagOffset(gpsIFDOffset + TIFF_HEADER_START, tag);
            if (tagEntryOffset > 0) {
                executeModifyAttributeValue(tagEntryOffset, value, findDataAreaOffset(subIFDOffset + TIFF_HEADER_START));
                isFound = true;
            }
        }
    }
    
    if (!isFound) { //没有找到，需要添加此属性
        return addAttribute(tag, value, dataType);
    }
    
    return EDIT_SUCCESS;
}

int ExifWriter::addAttribute(int16_t tag, void *value, uint16_t dataType) {
    uint8_t byteValue[4];
    memset(byteValue, 0, 4);
    
    if (dataType == TYPE_UINT8 || dataType == TYPE_INT8) {
        uint8_t tagValue = *((uint8_t *)value);
        Utils::convertInt8ToByteArray(tagValue, byteValue, alignIntel);
    } else if (dataType == TYPE_UINT16 || dataType == TYPE_INT16) {
        uint16_t tagValue = *((uint16_t *)value);
        Utils::convertInt16ToByteArray(tagValue, byteValue, alignIntel);
    } else if (dataType == TYPE_UINT32 || dataType == TYPE_INT32) {
        uint32_t tagValue = *((uint32_t *)value);
        Utils::convertInt32ToByteArray(tagValue, byteValue, alignIntel);
    } else if (dataType == TYPE_FLOAT) {
        float tagValue = *((float *)value);
        union {
            uint32_t i;
            float f;
        } i2f;
        i2f.f = tagValue;
        
        Utils::convertInt32ToByteArray(i2f.i, byteValue, alignIntel);
    } else if (dataType == TYPE_URATIONAL || dataType == TYPE_RATIONAL || dataType == TYPE_DOUBLE){ // double类型
        return executeAddDoubleAttribute(tag, (double *)value, dataType);
    } else if (dataType == TYPE_STRING) {
        return executeAddStringAttribute(tag, (std::string *)value, dataType);
    } else {
        return EDIT_CORRUPT_DATA;
    }
    
    //  组织数据
    uint8_t entryData[TIFF_ENTRY_LENGTH];
    memset(entryData, 0, sizeof(entryData));
    Utils::convertInt16ToByteArray(tag, entryData, alignIntel);
    Utils::convertInt16ToByteArray(dataType, entryData + 2, alignIntel);
    Utils::convertInt32ToByteArray(1, entryData + 4, alignIntel);
    memcpy(entryData + 8, byteValue, 4);
    
    // 修改entry数量
    uint16_t entryCount = Utils::parse16(buffer + TIFF_HEADER_START + TIFF_HEADER_LENGTH,alignIntel);
    Utils::convertInt16ToByteArray(entryCount + 1, byteValue, alignIntel);
    memcpy(buffer + TIFF_HEADER_START + TIFF_HEADER_LENGTH, byteValue, 2);
    
    // 将数据插入到第一个位置
    int entryDataStart = TIFF_HEADER_START + TIFF_HEADER_LENGTH + 2;
    expandBuffer(entryDataStart, TIFF_ENTRY_LENGTH, NULL, 0);
    memcpy(buffer + entryDataStart, entryData, TIFF_ENTRY_LENGTH);

    return EDIT_SUCCESS;
}

int ExifWriter::executeAddDoubleAttribute(int16_t tag, double *value, uint16_t dataType) {
    
    // 数据检查
    uint32_t offset = (TIFF_HEADER_START + TIFF_HEADER_LENGTH - 4);
    const unsigned first_ifd_offset = Utils::parse32(buffer + offset, alignIntel);
    offset += first_ifd_offset - 4; //数据起始于MM+002a
    if (offset + 2 > bufferLen) {
        return EDIT_CORRUPT_DATA;
    }
    
    // 将double转换为分数
    int numerator = 0;
    int denominator = 0;
    Utils::convertDoubleToFraction(*value, numerator, denominator);
    uint8_t bytes[8];
    Utils::convertInt32ToByteArray(numerator, bytes, alignIntel);
    Utils::convertInt32ToByteArray(denominator, bytes + 4, alignIntel);
    
    // 将数据写入数据区
    uint32_t dataAreaOffset = findDataAreaOffset(offset);
    expandBuffer(dataAreaOffset, 8, bytes, 8);
    
    //  组织数据
    uint8_t entryData[TIFF_ENTRY_LENGTH];
    memset(entryData, 0, sizeof(entryData));
    Utils::convertInt16ToByteArray(tag, entryData, alignIntel);
    Utils::convertInt16ToByteArray(dataType, entryData + 2, alignIntel);
    Utils::convertInt32ToByteArray(1, entryData + 4, alignIntel);
    Utils::convertInt32ToByteArray(dataAreaOffset - TIFF_HEADER_START, entryData + 8, alignIntel);
    
    // 修改entry数量
    uint16_t entryCount = Utils::parse16(buffer + TIFF_HEADER_START + TIFF_HEADER_LENGTH,alignIntel);
    Utils::convertInt16ToByteArray(entryCount + 1, bytes, alignIntel);
    memcpy(buffer + TIFF_HEADER_START + TIFF_HEADER_LENGTH, bytes, 2);
    
    // 将数据插入到第一个位置
    int entryDataStart = TIFF_HEADER_START + TIFF_HEADER_LENGTH + 2;
    expandBuffer(entryDataStart, TIFF_ENTRY_LENGTH, entryData, TIFF_ENTRY_LENGTH);
    
    return EDIT_SUCCESS;
}

int ExifWriter::executeAddStringAttribute(int16_t tag, std::string *value, uint16_t dataType) {
    
    if (value == NULL) {
        return EDIT_CORRUPT_DATA;
    }
    
    // 数据检查
    uint32_t offset = (TIFF_HEADER_START + TIFF_HEADER_LENGTH - 4);
    const unsigned first_ifd_offset = Utils::parse32(buffer + offset, alignIntel);
    offset += first_ifd_offset - 4; //数据起始于MM+002a
    if (offset + 2 > bufferLen) {
        return EDIT_CORRUPT_DATA;
    }
    
    // 长度大于4时，需要将数据写入数据区
    uint32_t strLen = (uint32_t)value->length();
    uint32_t dataAreaOffset = 0;
    if (strLen > 4) {
        dataAreaOffset = findDataAreaOffset(offset);
        expandBuffer(dataAreaOffset, strLen, (uint8_t *)(value->c_str()), strLen);
    }
    
    //  组织数据
    uint8_t entryData[TIFF_ENTRY_LENGTH];
    memset(entryData, 0, sizeof(entryData));
    Utils::convertInt16ToByteArray(tag, entryData, alignIntel);
    Utils::convertInt16ToByteArray(dataType, entryData + 2, alignIntel);
    Utils::convertInt32ToByteArray(strLen, entryData + 4, alignIntel);
    if (strLen > 4) {
        Utils::convertInt32ToByteArray(dataAreaOffset - TIFF_HEADER_START, entryData + 8, alignIntel);
    } else {
        memcpy(entryData + 8, (uint8_t *)value->c_str(), strLen);
    }
    
    // 修改entry数量
    uint16_t entryCount = Utils::parse16(buffer + TIFF_HEADER_START + TIFF_HEADER_LENGTH, alignIntel);
    uint8_t bytes[2];
    Utils::convertInt16ToByteArray(entryCount + 1, bytes, alignIntel);
    memcpy(buffer + TIFF_HEADER_START + TIFF_HEADER_LENGTH, bytes, 2);
    
    // 将数据插入到第一个位置
    int entryDataStart = TIFF_HEADER_START + TIFF_HEADER_LENGTH + 2;
    expandBuffer(entryDataStart, TIFF_ENTRY_LENGTH, entryData, TIFF_ENTRY_LENGTH);
    
    return EDIT_SUCCESS;
}

uint32_t ExifWriter::findTagOffset(uint32_t offset, int16_t tag) {
    int num_entries = Utils::parse16(buffer + offset, alignIntel);
    if (offset + 6 + 12 * num_entries > bufferLen) {
        return 0;
    }
    
    offset += 2;
    for (int i = 0; i < num_entries; i++) {
        int16_t curTag = Utils::parse16(buffer + offset, alignIntel);
        if (tag == curTag) {
            return offset;
        }
        offset += 12;
    }
    
    return 0;
}

uint32_t ExifWriter::findDataAreaOffset(uint32_t ifdDirectoryOffset) {
    int num_entries = Utils::parse16(buffer + ifdDirectoryOffset, alignIntel);
    return ifdDirectoryOffset + 2 + 12 * num_entries + 4;
}

// 修改某个Entry的值
void ExifWriter::executeModifyAttributeValue(uint32_t offset, void *value, uint32_t dataAreaOffset) {
    uint16_t dataType = Utils::parse16(buffer + offset + 2, alignIntel);
    switch (dataType) {
        case TYPE_UINT8:
        case TYPE_INT8:
        case TYPE_UINT16:
        case TYPE_INT16:
        case TYPE_UINT32:
        case TYPE_INT32:
            executeModifyAttributeIntValue(offset, (uint32_t *)value, dataAreaOffset);
            break;
            
        case TYPE_STRING:
            executeModifyAttributeStringValue(offset, (std::string *)value, dataAreaOffset);
            break;
            
        case TYPE_URATIONAL:
        case TYPE_RATIONAL:
        case TYPE_FLOAT:
        case TYPE_DOUBLE:
            executeModifyAttributeDoubleValue(offset, (double *)value, dataAreaOffset);
            break;
            
        default:
            break;
    }
}

void ExifWriter::executeModifyAttributeIntValue(uint32_t offset, uint32_t *value, uint32_t dataAreaOffset) {
    uint8_t byteValue[4];
    memset(byteValue, 0, 4);
    
    uint16_t dataType = Utils::parse16(buffer + offset + 2, alignIntel);
    if (dataType == TYPE_UINT8 || dataType == TYPE_INT8) {
        Utils::convertInt8ToByteArray(*value, byteValue, alignIntel);
    } else if (dataType == TYPE_UINT16 || dataType == TYPE_INT16) {
        Utils::convertInt16ToByteArray(*value, byteValue, alignIntel);
    } else {
        Utils::convertInt32ToByteArray(*value, byteValue, alignIntel);
    }
    
    memcpy(buffer + offset + 8, byteValue, 4 * sizeof(uint8_t)); //内存拷贝
}

void ExifWriter::executeModifyAttributeDoubleValue(uint32_t offset, double *value, uint32_t dataAreaOffset) {
    uint16_t dataType = Utils::parse16(buffer + offset + 2, alignIntel);
    if (dataType == TYPE_FLOAT) { // float类型的，直接写入data位置
        uint8_t byteValue[4];
        union {
            uint32_t i;
            float f;
        } i2f;
        i2f.f = (float) (*value);
        Utils::convertInt32ToByteArray(i2f.i, byteValue, alignIntel);
        memcpy(buffer + offset + 8, byteValue, 4 * sizeof(uint8_t)); //内存拷贝
    } else if (dataType == TYPE_DOUBLE) { // 不支持double
        return;
    } else if (dataType == TYPE_URATIONAL || dataType == TYPE_RATIONAL) {
        int numerator = 0;
        int denominator = 0;
        Utils::convertDoubleToFraction(*value, numerator, denominator);
        uint8_t bytes[8];
        Utils::convertInt32ToByteArray(numerator, bytes, alignIntel);
        Utils::convertInt32ToByteArray(denominator, bytes + 4, alignIntel);
        
        uint32_t contentOffset = Utils::parse32(buffer + offset + 8, alignIntel) + TIFF_HEADER_START;
        memcpy(buffer + contentOffset, bytes, 8);
    } else {
        return;
    }
}

void ExifWriter::executeModifyAttributeStringValue(uint32_t offset, std::string *value, uint32_t dataAreaOffset) {
    uint32_t componentCount = Utils::parse32(buffer + offset + 4, alignIntel);
    int32_t valueLen = (int32_t)value->length() + 1;
    
    // 修改components
    uint8_t byteValue[4];
    Utils::convertInt32ToByteArray(valueLen, byteValue, alignIntel);
    memcpy(buffer + offset + 4, byteValue, 4);
    
    if (componentCount <= 4) { // 数据直接写在了data区
        if (valueLen <= 4) { // 新值也小于4，直接在data内写
            memcpy(buffer + offset + 8, value->c_str(), valueLen);
            memset(buffer + offset + 8 + valueLen, 0, 1);
        } else { // 大于4个，需要在Data area区域扩展
            expandBuffer(dataAreaOffset, valueLen, NULL, 0);
            
            // 修改data的地址为offset地址
            uint8_t conentOffset[4];
            Utils::convertInt32ToByteArray(dataAreaOffset - TIFF_HEADER_START, conentOffset, alignIntel);
            memcpy(buffer + offset + 8, conentOffset, 4);
            
            // 修改value内容
            memcpy(buffer + dataAreaOffset, value->c_str(), valueLen);
        }
    } else {
        if (valueLen <= 4) { // 新值长度不足4，不用改数据，直接写入data
            memcpy(buffer + offset + 8, value->c_str(), valueLen);
            memset(buffer + offset + valueLen, 0, 1);
        } else {
            uint32_t contentOffset = Utils::parse32(buffer + offset + 8, alignIntel) + TIFF_HEADER_START;
            int32_t expandSize = valueLen - componentCount;
            expandBuffer(contentOffset, expandSize, NULL, 0);
            
            // 修改value的值
            memcpy(buffer + contentOffset, value->c_str(), valueLen);
        }
    }
}

void ExifWriter::expandBuffer(uint32_t start, int32_t expandSize, uint8_t *fillData, uint32_t fillDataSize) {
    if (expandSize == 0 || start > bufferLen) {
        return;
    } else {
        uint8_t *temp = new uint8_t[bufferLen + expandSize];
        memset(temp, 0, bufferLen + expandSize);
        memcpy(temp, buffer, start);
        memcpy(temp + start + expandSize, buffer + start, bufferLen - start);
        if (fillData != NULL) {
            memcpy(temp + start, fillData, fmin(expandSize, fillDataSize));
        }
        
        delete [] buffer;
        buffer = temp;
        bufferLen += expandSize;
    }
    
    // 修改data是offset的Entry
    int offset = TIFF_HEADER_START + TIFF_HEADER_LENGTH;
    increaseTagOffsetData(offset, start, expandSize);
    
    // 修改APP0的长度
    uint8_t byteData[2];
    uint16_t app0DataLen = Utils::parse16(buffer + 2, alignIntel);
    app0DataLen += expandSize;
    Utils::convertInt16ToByteArray(app0DataLen, byteData, alignIntel);
    memcpy(buffer + 2, byteData, 2);
}

void ExifWriter::increaseTagOffsetData(uint32_t offset, uint32_t startIndex, int32_t increaseSize) {
    int num_entries = Utils::parse16(buffer + offset, alignIntel);
    offset += 2;
    for (int i = 0; i < num_entries; i++) {
        int16_t curTag = Utils::parse16(buffer + offset, alignIntel);
        if (curTag == (int16_t) SUB_IFD_OFFSET || curTag == (int16_t)GPS_IFD_OFFSET) { // sub idf or gps ifd
            uint32_t value = Utils::parse32(buffer + offset + 8, alignIntel);
            if (value > startIndex) {
                value += increaseSize;
                executeModifyAttributeIntValue(offset, &value, 0);
            }
            increaseTagOffsetData(value + TIFF_HEADER_START, startIndex, increaseSize);
        } else {
            uint16_t dataType = Utils::parse16(buffer + offset + 2, alignIntel);
            uint32_t components = Utils::parse32(buffer + offset + 4, alignIntel);
            uint32_t valueSize = computeDataSize(dataType, components);
            if (valueSize > 4) {
                uint32_t value = Utils::parse32(buffer + offset + 8, alignIntel);
                if (value + TIFF_HEADER_START > startIndex) {
                    value += increaseSize;
                    executeModifyAttributeIntValue(offset, &value, 0);
                }
            }
        }
        
        offset += 12;
    }
}

uint32_t ExifWriter::computeDataSize(uint16_t dataType, uint32_t components) {
    switch (dataType) {
        case TYPE_UINT8: //unsigned byte
        case TYPE_STRING: //ascii strings
        case TYPE_INT8: // signed byte
        case TYPE_UNDEFINE: //undefined
            return components;
            
        case TYPE_UINT16: //unsigned short
        case TYPE_INT16: //signed short
            return 2 * components;
            
        case TYPE_UINT32: //unsigned long
        case TYPE_INT32: //signed long
        case TYPE_FLOAT: //single float
            return 4 * components;
            
        case TYPE_URATIONAL: //unsigned rational
        case TYPE_RATIONAL: //signed rational
        case TYPE_DOUBLE: //double float
            return 8 * components;
            
        default:
            break;
    }
    
    return 0;
}

uint8_t* ExifWriter::readExifData(const char *imagePath, uint32_t &len) {
    // 源文件读取流
    std::ifstream in (imagePath, std::ios::in | std::ios::binary);
    if (!in.is_open()) {
        return NULL;
    }
    
    uint8_t header[4];
    
    // 跳转到APP0结尾
    in.read((char*)header, 4);
    if (header[0] != JM_START || header[1] != JM_SOI || header[2] != JM_START || header[3] != JM_APP1) {
        // 头部4个字节不是FFD8 + FFE1，表示这个文件不是图片或者不包含EXIF信息
        in.close();
        return NULL;
    }
    
    uint8_t lenData[2];
    in.read((char *)lenData, 2);
    uint16_t exifLen = Utils::parse16(lenData, false);
    if (exifLen <= 0) {
        in.close();
        return NULL;
    }
    
    len = exifLen + 2;
    uint8_t *data = new uint8_t[len];
    in.seekg(2);
    if (!in.read((char *)data, len)) {
        in.close();
        return NULL;
    }
    
    in.close();
    
    return data;
}
}
