#include "hufcompressor.h"
#include <QBitArray>
#include <QDebug>
#include <QFile>

HUFCompressor::HUFCompressor(QObject *parent)
    : HuffmanBase{parent}, freqStats(256) {}

HUFCompressor::HUFCompressor(const QList<QString> &allFiles_, bool seperateKey_,
                             QObject *parent)
    : HuffmanBase{parent}, allFiles{allFiles_}, freqStats(256),
      seperateKey{seperateKey_} {}

// 清理数据
void HUFCompressor::init(const QList<QString> &allFiles_, bool seperateKey_) {
  allFiles = allFiles_;
  freqStats.clear();
  freqStats.resize(256);
  existByteCnt = 0;
  seperateKey = seperateKey_;
  maxByteFreq = 0;
}

// 生成单个文件的 FreqStats
void HUFCompressor::generateSingleFileFreqStats() {
  QString &sourcePath{allFiles[0]};
  QFile sourceFile{sourcePath};
  if (!sourceFile.open(QIODevice::ReadOnly)) {
    throw 801;
  }
  // 读取文件内容，统计字节词频
  long long bytesRead{};
  char buffer[readBufferSize]{};
  while (!sourceFile.atEnd()) {
    bytesRead += sourceFile.read(buffer, readBufferSize);
    for (int i = 0; i < bytesRead; i++) {
      freqStats[static_cast<uint8_t>(buffer[i])].second++;
    }
    bytesRead = 0;
  }
  // 补全 freqStats 数据，统计最大的词频，用于后续计算
  ByteSize byte{};
  for (int i = 0; i < 256; i++) {
    freqStats[byte].first = byte;
    ++byte;
    existByteCnt += freqStats[byte].second ? 1 : 0;
    maxByteFreq = std::max(maxByteFreq, freqStats[byte].second);
  }
}

// 将翻译后编码结果输出到文件
void HUFCompressor::appendHufCodeToSingleFile(const QString &outPath,
                                              const FileSize offByte) {
  QFile sourceFile{allFiles[0]};
  QFile outFile{outPath};
  sourceFile.open(QIODevice::ReadOnly);
  outFile.open(QIODevice::OpenModeFlag::Append);
  // 如果不使用密钥分离，Key 此时已经被写入 outFile，
  // 需要通过 offByte 偏移得到文件数据的起始写入位置
  if (!seperateKey) {
    outFile.seek(offByte);
  }

  // 读取文件
  long long bytesRead{};
  char buffer[1024]{};
  char writeBuffer[2]{};
  int writeBufferLength{};
  // 读取到 buffer 中，按字节处理，
  // 每个字节被翻译为对应的 HuffmanCode，
  // 通过位运算维护一个 writeBuffer ，和一个记录 writeBuffer 当前有效位数的
  // writeBufferLength， Length 达到 8，即达到 1 Byte 立刻写入 outFile
  while (!sourceFile.atEnd()) {
    bytesRead += sourceFile.read(buffer, readBufferSize);
    for (int i = 0; i < bytesRead; i++) {
      HuffmanCode cur{getHufCode(static_cast<uint8_t>(buffer[i]))};
      // BitArray 的每个 uint64_t 都需处理
      for (int j = 0; j < cur.bitLength; j++) {
        int index{j / 64};
        // 按位读入 writeBuffer
        if (cur.huf.arr[index] & 1) {
          writeBuffer[0] |= (1 << writeBufferLength);
        }
        cur.huf.arr[index] >>= 1;
        ++writeBufferLength;
        // 达到 1 Byte 立刻写入
        if (writeBufferLength == 8) {
          outFile.write(writeBuffer, 1);
          writeBuffer[0] = 0;
          writeBufferLength = 0;
        }
      }
    }
    // buffer 处理结束，刷新文件读写缓冲区，
    // 并重设 bytesRead 计数器
    bytesRead = 0;
    outFile.flush();
  }
  // 最后没有凑成 1 Byte，也需要写入
  if (writeBufferLength) {
    outFile.write(writeBuffer, 1);
  }
}

// 将字节词频数据写入 keyFile
FileSize HUFCompressor::appendHufKeyToSingleFile(const QString &keyPath) {
  const char header[5]{"HUFC"};
  QFile keyFile{keyPath};
  if (!keyFile.open(QIODevice::OpenModeFlag::Append)) {
    throw 802;
  }
  // 写入文件头 HUFC
  keyFile.write(header, 4);
  char cnt[2]{};
  // 注意，existByteCnt 最大可为 256，超出 uint8_t 范围
  // 减 1 后存储，因为 existByteCnt 不可能为 0
  cnt[0] = static_cast<uint8_t>(existByteCnt - 1);
  keyFile.write(cnt, 1);

  // 计算需要几位存储每个字节的 ByteFreq
  int byteCntForFreq{};
  FreqSize tmp{maxByteFreq};
  while (tmp) {
    tmp >>= 8;
    ++byteCntForFreq;
  }
  char fcnt[2]{};
  fcnt[0] = static_cast<uint8_t>(byteCntForFreq);
  keyFile.write(fcnt, 1);

  // 写入每个 std::pair<ByteSize, FreqSize>
  char writeBuffer[2048]{};
  int curIndex{};
  for (int i = 0; i < freqStats.size(); i++) {
    if (freqStats[i].second) {
      writeBuffer[curIndex] = freqStats[i].first;
      // 多字节存储需要位操作分开计算
      for (int j = 0; j < byteCntForFreq; j++) {
        writeBuffer[curIndex + 1 + j] =
            static_cast<uint8_t>(freqStats[i].second >> (8 * j));
      }
      curIndex += 1 + byteCntForFreq;
    }
  }
  keyFile.write(writeBuffer, (1 + byteCntForFreq) * existByteCnt);
  // 返回上述字节词频数据使用的字节大小
  FileSize processedBytes{4u + 2 + curIndex};
  return processedBytes;
}

// 压缩单个文件
void HUFCompressor::compressSingleFile(const QString &outPath,
                                       const QString &keyPath) {
  generateSingleFileFreqStats();
  buildTree(this->freqStats);
  buildHashMap();
  FileSize processed{appendHufKeyToSingleFile(keyPath)};
  appendHufCodeToSingleFile(outPath, processed);
}
