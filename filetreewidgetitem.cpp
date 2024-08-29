#include "filetreewidgetitem.h"
#include <QFileInfo>

FileTreeWidgetItem::FileTreeWidgetItem() {}

// 用于解决文件大小排序，统一 Byte 单位排序而不是字典序
bool FileTreeWidgetItem::operator<(const QTreeWidgetItem &other) const {
  int sortByColumn{this->treeWidget()->sortColumn()};
  constexpr int sizeColumn{2};
  constexpr int pathColumn{3};
  QFileInfo currentInfo{this->text(pathColumn)};
  QFileInfo otherInfo{other.text(pathColumn)};
  // 标记文件缺失错误，若错误则标记，并修改图标名称
  if (!currentInfo.exists() && this->text(6) == "0") {
    FileTreeWidgetItem *editThis{const_cast<FileTreeWidgetItem *>(this)};
    editThis->setText(6, "1");
    editThis->setText(1, "*" + this->text(1));
    editThis->setIcon(0, QIcon{":/fileIcons/resources/x.ico"});
  }
  if (!otherInfo.exists() && other.text(6) == "0") {
    QTreeWidgetItem *editOther{const_cast<QTreeWidgetItem *>(&other)};
    editOther->setText(6, "1");
    editOther->setText(1, "*" + other.text(1));
    editOther->setIcon(0, QIcon{":/fileIcons/resources/x.ico"});
  }
  // 按 Byte 单位排序
  if (sortByColumn == sizeColumn) {
    return currentInfo.size() < otherInfo.size();
  }
  // 其他情况按字典序排序
  return this->text(sortByColumn) < other.text(sortByColumn);
}
