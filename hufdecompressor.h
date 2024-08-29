#pragma once

#include "huffmanbase.h"
#include <QFile>
#include <QObject>

constexpr int readBufferSize_forKey{2048};
constexpr int readBufferSize_forHufCode{1024};

class HUFDecompressor : public HuffmanBase {
  Q_OBJECT
  // Members
private:
  QList<QString> allFiles{};
  QList<std::pair<ByteSize, FreqSize>> freqStats;
  bool seperateKey{};

  // Construction Functions
public:
  explicit HUFDecompressor(QObject *parent = nullptr);
  explicit HUFDecompressor(const QList<QString> &allFiles_, bool seperateKey_,
                           QObject *parent = nullptr);

  // Functions
public:
  void init(const QList<QString> &allFiles_, bool seperateKey_);

  // 解压单个文件
  void decompressSingleFile(const QString &outPath, const QString &keyPath);

private:
  // 从单文件中读取字节词频数据
  FileSize readSingleFileFreqStats(const QString &keyPath);

  // 将翻译后文件结果写入输出文件
  void appendOutDataToSingleFile(const QString &outPath, FileSize offByte = 0);
};
