# WriterableTinyExif

目前网上有很多读Jpeg Exif信息的库，但修改和添加exif信息的库比较少。TinyExif库是一个比较轻量级的Exif信息读取库，但它只有读的功能。WriterableTinyExif是在tinyExif的基础上进一步扩展修改Exif属性的功能。

``` c++

// 创建一个EXIFInfo对象，设置要修改或添加的值
TinyEXIF::EXIFInfo imageEXIF2;
imageEXIF2.ImageWidth = 1984;
imageEXIF2.ImageHeight = 1488;
imageEXIF2.RelatedImageWidth = 1984;
imageEXIF2.RelatedImageWidth = 1488;
imageEXIF2.Software = imageEXIF.Software.append("#zsy-1");

// 从一个图片中读出当前的EXIF的数据
uint32_t exifDataLen = 0;
uint8_t *exifData = TinyEXIF::ExifWriter::readExifData(argv[1], exifDataLen);
TinyEXIF::ExifWriter writer(exifData, exifDataLen); //使用这个数据创建一个ExifWriter
//TinyEXIF::ExifWriter writer; //也可以直接创建一个空数据的exifWriter
delete exifData;

// 在writer中添加要修改的exifInfo信息    
writer.addExifInfo(&imageEXIF2);
// 将图片argv[1]修改exif后输出到argv[2]
writer.writeToFile(argv[1], argv[2]);
```

这可库也可扩展到Android和IOS工程中使用。后面我封装后补充一下相关代码。