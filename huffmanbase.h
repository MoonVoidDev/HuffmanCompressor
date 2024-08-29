#pragma once

#include <QHash>
#include <QObject>

using ByteSize = uint8_t; // ByteSize 不可修改
using FreqSize = uint32_t;
using NodeCntSize = uint32_t;
using DepthSize = uint32_t;
using FileSize = uint32_t;

// Huffman 树中的节点 Class
struct HuffmanNode {
  bool isLeaveNode{};
  ByteSize byteCode{};    // or uint8_t
  FreqSize frequency{};   // or uint32_t
  NodeCntSize curIndex{}; // or uint16_t
  // 节点深度
  DepthSize depth{};
  // 左子节点索引，分配边权 0
  NodeCntSize leftChildIndex{};
  // 右子节点索引，分配边权 1
  NodeCntSize rightChildIndex{};
  HuffmanNode(bool isLeaveNode_, ByteSize byteCode_, FreqSize frequency_,
              NodeCntSize curIndex_, DepthSize depth_)
      : isLeaveNode{isLeaveNode_}, byteCode{byteCode_}, frequency{frequency_},
        curIndex{curIndex_}, depth{depth_} {}
  // 用于小根堆的比较函数
  bool operator>(const HuffmanNode &other) const;
};

// 用于记录一个 Huffman 编码的 Class
struct HuffmanCode {
  // 编码长度
  ByteSize bitLength{};
  // 编码值，极限情况长度可达 255，使用 4 个 uint64_t
  struct BitArray {
    uint64_t arr[4]{};
    bool operator==(const BitArray &other) const;
  } huf{};
  HuffmanCode() : bitLength{} {}
  // 用于 QHash 计算 hash 值的散列函数
  friend size_t qHash(const HuffmanCode &code, size_t seed);
  // 向此 HuffmanCode 后添加一位 0/1
  HuffmanCode append(bool bit);
  // 用于 QHash 计算的等于比较函数
  bool operator==(const HuffmanCode &other) const;
};

// 使用 HuffmanNode 数组表示 Huffman 树
using HuffmanTree = QList<HuffmanNode>;

// Huffman 编码的运算核心 Class
class HuffmanBase : public QObject {
  Q_OBJECT
  Q_CLASSINFO("author", "huang")
  Q_CLASSINFO("version", "1.0")

  // Members
private:
  NodeCntSize treeNodeCount{}; // 记录树中总节点数
  bool lock1{};                // 防止错误使用此 Class 的标记
  HuffmanTree tree{};          // Huffman 树
  bool lock2{};                // 防错标记
  QHash<ByteSize, HuffmanCode>
      byteToHufCode{}; // 字节数据 到 HuffmanCode 的映射
  QHash<HuffmanCode, ByteSize>
      hufCodeToByte{}; // HuffmanCode 到 字节数据 的映射

  // Construction Functions
public:
  explicit HuffmanBase(QObject *parent = nullptr);

  // Functions
public:
  // 根据字节词频数据，建立 Huffman 树
  void buildTree(const QList<std::pair<ByteSize, FreqSize>> input);
  // 遍历树，用于获取每个叶子节点的 HuffmanCode，并建立双向映射
  void traverse(NodeCntSize curIndex, HuffmanCode curHufCode);
  // 建立映射
  void buildHashMap();

  // Get 接口
  HuffmanCode getHufCode(ByteSize byteData) const {
    return byteToHufCode[byteData];
  }
  ByteSize getByteData(const HuffmanCode &hufCode, bool *ok) const {
    if (hufCodeToByte.contains(hufCode)) {
      *ok = true;
      return hufCodeToByte[hufCode];
    } else {
      *ok = false;
      return 0;
    }
  }

signals:
};
