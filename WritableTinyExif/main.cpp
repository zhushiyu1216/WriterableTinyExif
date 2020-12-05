//
//  main.cpp
//  WritableTinyExif
//
//  Created by zhushiyu01 on 2020/11/29.
//

// Defines the entry point for the console application.

#ifdef _MSC_VER
#include <windows.h>
#endif
#include "TinyEXIF.h"
#include "TinyExifWriter.hpp"
#include "Utils.h"

#include <iostream> // std::cout
#include <fstream>  // std::ifstream
#include <vector>   // std::vector
#include <iomanip>  // std::setprecision

class EXIFStreamFile : public TinyEXIF::EXIFStream {
public:
    explicit EXIFStreamFile(const char* fileName)
        : file(fileName, std::ifstream::in|std::ifstream::binary) {}
    bool IsValid() const override {
        return file.is_open();
    }
    const uint8_t* GetBuffer(unsigned desiredLength) override {
        buffer.resize(desiredLength);
        if (!file.read((char*)buffer.data(), desiredLength))
            return NULL;
        return buffer.data();
    }
    bool SkipBuffer(unsigned desiredLength) override {
        return (bool)file.seekg(desiredLength,std::ios::cur);
    }
private:
    std::ifstream file;
    std::vector<uint8_t> buffer;
};


/// 测试添加、修改exif属性。
/// argv[1] 是想要复制exif属性的图片地址
/// argv[2] 是要修改的jpeg图片地址
/// argv[3]是要输出的jpeg图片地址
/// @param argv <#argv description#>
void testModifyExif(const char** argv) {
//    EXIFStreamFile stream(argv[1]);
//    TinyEXIF::EXIFInfo imageEXIF(stream);
    
    TinyEXIF::EXIFInfo imageEXIF2;
//    imageEXIF2.ImageWidth = 1984;
//    imageEXIF2.ImageHeight = 1488;
//    imageEXIF2.RelatedImageWidth = 1984;
//    imageEXIF2.RelatedImageWidth = 1488;
//    imageEXIF2.Software = imageEXIF.Software.append("#zsy-1");
////    imageEXIF2.Software = "#1";
    imageEXIF2.DateTimeOriginal = "2020";
//    imageEXIF2.ExposureProgram = 1;
//    imageEXIF2.GeoLocation.GPSDateStamp = "2020:10:24---";
//    imageEXIF2.FNumber = 2.8;
//    imageEXIF2.ApertureValue = 2.79;
    
    uint32_t exifDataLen = 0;
    uint8_t *exifData = TinyEXIF::ExifWriter::readExifData(argv[1], exifDataLen);
//    TinyEXIF::ExifWriter writer(exifData, exifDataLen);
    TinyEXIF::ExifWriter writer;
    delete exifData;
    
    writer.addExifInfo(&imageEXIF2);
    writer.writeToFile(argv[2], argv[3]);
}

int main(int argc, const char** argv)
{
    if (argc < 2) {
        std::cout << "Usage: TinyEXIF <image_file>\n";
        return -1;
    }
    
    std::cout << "input file:" << argv[1] << "\n";

    testModifyExif(argv);
    
    // read entire image file
    EXIFStreamFile stream(argv[3]);
    if (!stream.IsValid()) {
        std::cout << "error: can not open '" << argv[1] << "'\n";
        return -2;
    }
    
    std::cout << "write file:" << argv[3] << "\n";

    // parse image EXIF and XMP metadata
    TinyEXIF::EXIFInfo imageEXIF(stream);
    if (!imageEXIF.Fields) {
        std::cout << "error: no EXIF or XMP metadata\n";
        return -3;
    }

    // print extracted metadata
    if (imageEXIF.ImageWidth || imageEXIF.ImageHeight)
        std::cout << "ImageResolution " << imageEXIF.ImageWidth << "x" << imageEXIF.ImageHeight << " pixels" << "\n";
    if (imageEXIF.RelatedImageWidth || imageEXIF.RelatedImageHeight)
        std::cout << "RelatedImageResolution " << imageEXIF.RelatedImageWidth << "x" << imageEXIF.RelatedImageHeight << " pixels" << "\n";
    if (!imageEXIF.ImageDescription.empty())
        std::cout << "Description " << imageEXIF.ImageDescription << "\n";
    if (!imageEXIF.Make.empty() || !imageEXIF.Model.empty())
        std::cout << "CameraModel " << imageEXIF.Make << " - " << imageEXIF.Model << "\n";
    if (!imageEXIF.SerialNumber.empty())
        std::cout << "SerialNumber " << imageEXIF.SerialNumber << "\n";
    if (imageEXIF.Orientation)
        std::cout << "Orientation " << imageEXIF.Orientation << "\n";
    if (imageEXIF.XResolution || imageEXIF.YResolution || imageEXIF.ResolutionUnit)
        std::cout << "Resolution " << imageEXIF.XResolution << "x" << imageEXIF.YResolution << " (" << imageEXIF.ResolutionUnit << ")\n";
    if (imageEXIF.BitsPerSample)
        std::cout << "BitsPerSample " << imageEXIF.BitsPerSample << "\n";
    if (!imageEXIF.Software.empty())
        std::cout << "Software " << imageEXIF.Software << "\n";
    if (!imageEXIF.DateTime.empty())
        std::cout << "DateTime " << imageEXIF.DateTime << "\n";
    if (!imageEXIF.DateTimeOriginal.empty())
        std::cout << "DateTimeOriginal " << imageEXIF.DateTimeOriginal << "\n";
    if (!imageEXIF.DateTimeDigitized.empty())
        std::cout << "DateTimeDigitized " << imageEXIF.DateTimeDigitized << "\n";
    if (!imageEXIF.SubSecTimeOriginal.empty())
        std::cout << "SubSecTimeOriginal " << imageEXIF.SubSecTimeOriginal << "\n";
    if (!imageEXIF.Copyright.empty())
        std::cout << "Copyright " << imageEXIF.Copyright << "\n";
    std::cout << "ExposureTime " << std::setprecision(10) << imageEXIF.ExposureTime << " s" << "\n";
    std::cout << "FNumber " << imageEXIF.FNumber << "\n";
    std::cout << "ExposureProgram " << imageEXIF.ExposureProgram << "\n";
    std::cout << "ISOSpeed " << imageEXIF.ISOSpeedRatings << "\n";
    std::cout << "ShutterSpeedValue " << std::setprecision(10) << imageEXIF.ShutterSpeedValue << "\n";
    std::cout << "ApertureValue " << std::setprecision(10) << imageEXIF.ApertureValue << "\n";
    std::cout << "BrightnessValue " << std::setprecision(10) << imageEXIF.BrightnessValue << "\n";
    std::cout << "ExposureBiasValue " << imageEXIF.ExposureBiasValue << "\n";
    std::cout << "SubjectDistance " << imageEXIF.SubjectDistance << "\n";
    std::cout << "FocalLength " << imageEXIF.FocalLength << " mm" << "\n";
    std::cout << "Flash " << imageEXIF.Flash << "\n";
    if (!imageEXIF.SubjectArea.empty()) {
        std::cout << "SubjectArea";
        for (uint16_t val: imageEXIF.SubjectArea)
            std::cout << " " << val;
        std::cout << "\n";
    }
    std::cout << "MeteringMode " << imageEXIF.MeteringMode << "\n";
    std::cout << "LightSource " << imageEXIF.LightSource << "\n";
    std::cout << "ProjectionType " << imageEXIF.ProjectionType << "\n";
    if (imageEXIF.Calibration.FocalLength != 0)
        std::cout << "Calibration.FocalLength " << imageEXIF.Calibration.FocalLength << " pixels" << "\n";
    if (imageEXIF.Calibration.OpticalCenterX != 0)
        std::cout << "Calibration.OpticalCenterX " << imageEXIF.Calibration.OpticalCenterX << " pixels" << "\n";
    if (imageEXIF.Calibration.OpticalCenterY != 0)
        std::cout << "Calibration.OpticalCenterY " << imageEXIF.Calibration.OpticalCenterY << " pixels" << "\n";
    std::cout << "LensInfo.FStopMin " << imageEXIF.LensInfo.FStopMin << "\n";
    std::cout << "LensInfo.FStopMax " << imageEXIF.LensInfo.FStopMax << "\n";
    std::cout << "LensInfo.FocalLengthMin " << imageEXIF.LensInfo.FocalLengthMin << " mm" << "\n";
    std::cout << "LensInfo.FocalLengthMax " << imageEXIF.LensInfo.FocalLengthMax << " mm" << "\n";
    std::cout << "LensInfo.DigitalZoomRatio " << imageEXIF.LensInfo.DigitalZoomRatio << "\n";
    std::cout << "LensInfo.FocalLengthIn35mm " << imageEXIF.LensInfo.FocalLengthIn35mm << "\n";
    std::cout << "LensInfo.FocalPlaneXResolution " << std::setprecision(10) << imageEXIF.LensInfo.FocalPlaneXResolution << "\n";
    std::cout << "LensInfo.FocalPlaneYResolution " << std::setprecision(10) << imageEXIF.LensInfo.FocalPlaneYResolution << "\n";
    std::cout << "LensInfo.FocalPlaneResolutionUnit " << imageEXIF.LensInfo.FocalPlaneResolutionUnit << "\n";
    if (!imageEXIF.LensInfo.Make.empty() || !imageEXIF.LensInfo.Model.empty())
        std::cout << "LensInfo.Model " << imageEXIF.LensInfo.Make << " - " << imageEXIF.LensInfo.Model << "\n";
    if (imageEXIF.GeoLocation.hasLatLon()) {
        std::cout << "GeoLocation.Latitude " << std::setprecision(10) << imageEXIF.GeoLocation.Latitude << "\n";
        std::cout << "GeoLocation.Longitude " << std::setprecision(10) << imageEXIF.GeoLocation.Longitude << "\n";
    }
    if (imageEXIF.GeoLocation.hasAltitude()) {
        std::cout << "GeoLocation.Altitude " << imageEXIF.GeoLocation.Altitude << " m" << "\n";
        std::cout << "GeoLocation.AltitudeRef " << (int)imageEXIF.GeoLocation.AltitudeRef << "\n";
    }
    if (imageEXIF.GeoLocation.hasRelativeAltitude())
        std::cout << "GeoLocation.RelativeAltitude " << imageEXIF.GeoLocation.RelativeAltitude << " m" << "\n";
    if (imageEXIF.GeoLocation.hasOrientation()) {
        std::cout << "GeoLocation.RollDegree " << imageEXIF.GeoLocation.RollDegree << "\n";
        std::cout << "GeoLocation.PitchDegree " << imageEXIF.GeoLocation.PitchDegree << "\n";
        std::cout << "GeoLocation.YawDegree " << imageEXIF.GeoLocation.YawDegree << "\n";
    }
    if (imageEXIF.GeoLocation.hasSpeed()) {
        std::cout << "GeoLocation.SpeedX " << imageEXIF.GeoLocation.SpeedX << " m/s" << "\n";
        std::cout << "GeoLocation.SpeedY " << imageEXIF.GeoLocation.SpeedY << " m/s" << "\n";
        std::cout << "GeoLocation.SpeedZ " << imageEXIF.GeoLocation.SpeedZ << " m/s" << "\n";
    }
    if (imageEXIF.GeoLocation.AccuracyXY > 0 || imageEXIF.GeoLocation.AccuracyZ > 0)
        std::cout << "GeoLocation.GPSAccuracy XY " << imageEXIF.GeoLocation.AccuracyXY << " m" << " Z " << imageEXIF.GeoLocation.AccuracyZ << " m" << "\n";
    std::cout << "GeoLocation.GPSDOP " << imageEXIF.GeoLocation.GPSDOP << "\n";
    std::cout << "GeoLocation.GPSDifferential " << imageEXIF.GeoLocation.GPSDifferential << "\n";
    if (!imageEXIF.GeoLocation.GPSMapDatum.empty())
        std::cout << "GeoLocation.GPSMapDatum " << imageEXIF.GeoLocation.GPSMapDatum << "\n";
    if (!imageEXIF.GeoLocation.GPSTimeStamp.empty())
        std::cout << "GeoLocation.GPSTimeStamp " << imageEXIF.GeoLocation.GPSTimeStamp << "\n";
    if (!imageEXIF.GeoLocation.GPSDateStamp.empty())
        std::cout << "GeoLocation.GPSDateStamp " << imageEXIF.GeoLocation.GPSDateStamp << "\n";
    return EXIT_SUCCESS;
}
