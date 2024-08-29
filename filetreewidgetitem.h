#pragma once

#include <QObject>
#include <QTreeWidgetItem>
#include <QWidget>

// 用于视图树的 Item Class，重载了 operator< 用于排序
class FileTreeWidgetItem : public QTreeWidgetItem {

private:
  QList<FileTreeWidgetItem *> missed{};

public:
  FileTreeWidgetItem();
  FileTreeWidgetItem(QTreeWidget *treeview, int type)
      : QTreeWidgetItem{treeview, type} {}
  FileTreeWidgetItem(QTreeWidgetItem *parent, int type)
      : QTreeWidgetItem{parent, type} {}
  virtual ~FileTreeWidgetItem() {}

  bool operator<(const QTreeWidgetItem &other) const override;
};
