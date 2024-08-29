#include "hufdecompressor.h"
#include <QDebug>
#include <QDir>
#include <QFile>

HUFDecompressor::HUFDecompressor(QObject *parent)
    : HuffmanBase{parent}, freqStats(256) {}
HUFDecompressor::HUFDecompressor(const QList<QString> &allFiles_,
                                 bool seperateKey_, QObject *parent)
    : HuffmanBase{parent}, allFiles{allFiles_}, freqStats(256),
      seperateKey{seperateKey_} {}

// 清理数据
void HUFDecompressor::init(const QList<QString> &allFiles_, bool seperateKey_) {
  allFiles = allFiles_;
  freqStats.clear();
  freqStats.resize(256);
  seperateKey = seperateKey_;
}

// 从 keyFile 读取词频数据
FileSize HUFDecompressor::readSingleFileFreqStats(const QString &keyPath) {
  QFile keyFile{keyPath};
  if (!keyFile.open(QIODevice::OpenModeFlag::ReadOnly)) {
    throw 701;
  }
  char buffer[readBufferSize_forKey]{};
  // 检查文件类型
  keyFile.read(buffer, 4);
  if (strcmp(buffer, "HUFC")) {
    throw 702;
  }
  // 读取 Key 组成结构，此处获得有多少对 pair，每个 pair 的 Freq 占多少字节
  keyFile.read(buffer, 2);
  int pairCnt{static_cast<ByteSize>(buffer[0]) + 1};
  int byteCntForFreq{static_cast<ByteSize>(buffer[1])};

  // 读取 pair
  keyFile.read(buffer, (1 + byteCntForFreq) * pairCnt);
  int curIndex{};
  for (int i = 0; i < pairCnt; i++) {
    ByteSize curByte{static_cast<ByteSize>(buffer[curIndex])};
    FreqSize curFreq{};
    for (int j = 0; j < byteCntForFreq; j++) {
      // 注意此处转化，需要先转为 uint8_t 否则 char 会符号扩展发生错误
      curFreq += ((
          static_cast<FreqSize>(static_cast<ByteSize>(buffer[curIndex + 1 + j]))
          << (8 * j)));
    }
    freqStats[curByte].first = curByte;
    freqStats[curByte].second = curFreq;
    curIndex += 1 + byteCntForFreq;
  }
  // 返回处理的字节数量
  FileSize processedBytes{4u + 2 + (1 + byteCntForFreq) * pairCnt};
  return processedBytes;
}

// 将翻译后的文件数据写入 outFile
void HUFDecompressor::appendOutDataToSingleFile(const QString &outPath,
                                                const FileSize offByte) {

  QFile sourceFile{allFiles[0]};
  QFile outFile{outPath};
  if (!sourceFile.open(QIODevice::OpenModeFlag::ReadOnly) ||
      !outFile.open(QIODevice::OpenModeFlag::Append)) {
    throw 703;
  }
  // 设置偏移
  if (!seperateKey) {
    sourceFile.seek(offByte);
  }
  // 读取源文件用于翻译其 Huffman 编码
  long long bytesRead{};
  char buffer[readBufferSize_forHufCode]{};
  char writeBuffer[2]{};
  // 按位读取，每次读取一位都从 getByteData() 接口查询是否是有效的 HuffmanCode，
  // 如果有效则将字节立刻写入 outFile，并清空当前构造的 HuffmanCode
  HuffmanCode curRead{};
  while (!sourceFile.atEnd()) {
    bytesRead += sourceFile.read(buffer + bytesRead, readBufferSize_forHufCode);
    for (int i = 0; i < bytesRead; i++) {
      ByteSize curByte{static_cast<ByteSize>(buffer[i])};
      for (int j = 0; j < 8; j++) {
        // 按位构造 HuffmanCode
        curRead.append(curByte & 1);
        curByte >>= 1;
        bool ok{};
        ByteSize data{getByteData(curRead, &ok)};
        if (ok) {
          // 有效编码，执行翻译输出
          writeBuffer[0] = data;
          outFile.write(writeBuffer, 1);
          curRead = {};
        }
      }
    }
    bytesRead = 0;
    outFile.flush();
  }
}

// 解压单个文件
void HUFDecompressor::decompressSingleFile(const QString &outPath,
                                           const QString &keyPath) {
  FileSize processed{readSingleFileFreqStats(keyPath)};
  buildTree(this->freqStats);
  buildHashMap();
  appendOutDataToSingleFile(outPath, processed);
}
