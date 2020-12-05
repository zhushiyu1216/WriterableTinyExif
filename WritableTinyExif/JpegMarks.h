//
//  JpegMarks.h
//  WritableTinyExif
//
//  Created by zhushiyu01 on 2020/11/29.
//

#ifndef JpegMarks_h
#define JpegMarks_h

enum JPEG_MARKERS {
    JM_START = 0xFF,
    JM_SOF0  = 0xC0,
    JM_SOF1  = 0xC1,
    JM_SOF2  = 0xC2,
    JM_SOF3  = 0xC3,
    JM_DHT   = 0xC4,
    JM_SOF5  = 0xC5,
    JM_SOF6  = 0xC6,
    JM_SOF7  = 0xC7,
    JM_JPG   = 0xC8,
    JM_SOF9  = 0xC9,
    JM_SOF10 = 0xCA,
    JM_SOF11 = 0xCB,
    JM_DAC   = 0xCC,
    JM_SOF13 = 0xCD,
    JM_SOF14 = 0xCE,
    JM_SOF15 = 0xCF,
    JM_RST0  = 0xD0,
    JM_RST1  = 0xD1,
    JM_RST2  = 0xD2,
    JM_RST3  = 0xD3,
    JM_RST4  = 0xD4,
    JM_RST5  = 0xD5,
    JM_RST6  = 0xD6,
    JM_RST7  = 0xD7,
    JM_SOI   = 0xD8,
    JM_EOI   = 0xD9,
    JM_SOS   = 0xDA,
    JM_DQT     = 0xDB,
    JM_DNL     = 0xDC,
    JM_DRI     = 0xDD,
    JM_DHP     = 0xDE,
    JM_EXP     = 0xDF,
    JM_APP0  = 0xE0,
    JM_APP1  = 0xE1, // EXIF and XMP
    JM_APP2  = 0xE2,
    JM_APP3  = 0xE3,
    JM_APP4  = 0xE4,
    JM_APP5  = 0xE5,
    JM_APP6  = 0xE6,
    JM_APP7  = 0xE7,
    JM_APP8  = 0xE8,
    JM_APP9  = 0xE9,
    JM_APP10 = 0xEA,
    JM_APP11 = 0xEB,
    JM_APP12 = 0xEC,
    JM_APP13 = 0xED, // IPTC
    JM_APP14 = 0xEE,
    JM_APP15 = 0xEF,
    JM_JPG0     = 0xF0,
    JM_JPG1     = 0xF1,
    JM_JPG2     = 0xF2,
    JM_JPG3     = 0xF3,
    JM_JPG4     = 0xF4,
    JM_JPG5     = 0xF5,
    JM_JPG6     = 0xF6,
    JM_JPG7     = 0xF7,
    JM_JPG8     = 0xF8,
    JM_JPG9     = 0xF9,
    JM_JPG10 = 0xFA,
    JM_JPG11 = 0xFB,
    JM_JPG12 = 0xFC,
    JM_JPG13 = 0xFD,
    JM_COM   = 0xFE
};

// APP0内的图片TAG值
enum JPEG_EXIF_TAG {
    IMAGE_WIDTH             = 0xa002,
    IMAGE_HEIGHT            = 0xa003,
    IMAGE_RELATIVED_WIDTH   = 0x1001,
    IMAGE_RELATIVED_HEIGHT  = 0x1002,
    IMAGE_SOFTWARE          = 0x0131,
    SUB_IFD_OFFSET          = 0x8769,
    
    DATETIME_ORIGINAL       = 0x9003,
    EXPOSURE_PROGRAM        = 0x8822,
    
    GPS_IFD_OFFSET          = 0x8825,
    GEO_DATE_STAMP          = 0x1D,
    
    IFD_FNUMBER             = 0x829d,
    IFD_APERTUREVALUE       = 0x9202,
};

const uint16_t TYPE_UINT8           = 1;
const uint16_t TYPE_STRING          = 2;
const uint16_t TYPE_UINT16          = 3;
const uint16_t TYPE_UINT32          = 4;
const uint16_t TYPE_URATIONAL       = 5;
const uint16_t TYPE_INT8            = 6;
const uint16_t TYPE_UNDEFINE        = 7;
const uint16_t TYPE_INT16           = 8;
const uint16_t TYPE_INT32           = 9;
const uint16_t TYPE_RATIONAL        = 10;
const uint16_t TYPE_FLOAT           = 11;
const uint16_t TYPE_DOUBLE          = 12;

#endif /* JpegMarks_h */
