#pragma once

#include "huffmanbase.h"

#include <QObject>

constexpr int readBufferSize{1024};

class HUFCompressor : public HuffmanBase {
  Q_OBJECT

  // Members
private:
  QList<QString> allFiles{};
  QList<std::pair<ByteSize, FreqSize>> freqStats;
  int existByteCnt{};
  bool seperateKey{};
  FreqSize maxByteFreq{};

  // Construction Functions
public:
  explicit HUFCompressor(QObject *parent = nullptr);
  explicit HUFCompressor(const QList<QString> &allFiles_, bool seperateKey_,
                         QObject *parent = nullptr);

  // Functions
public:
  void init(const QList<QString> &allFiles_, bool seperateKey_);

  // 压缩单个文件
  void compressSingleFile(const QString &outPath, const QString &keyPath);

private:
  // 生成单文件的字节词频数据
  void generateSingleFileFreqStats();

  // 向单文件写入 Huffman 编码结果
  void appendHufCodeToSingleFile(const QString &outPath,
                                 const FileSize offByte);

  // 向单文件写入字节词频数据
  FileSize appendHufKeyToSingleFile(const QString &keyPath);
};
