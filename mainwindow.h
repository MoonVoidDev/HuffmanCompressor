#pragma once

#include "filetreewidgetitem.h"
#include "utils.h"

#include <QDir>
#include <QMainWindow>
#include <QTreeWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private:
  FileTreeWidgetItem *addNewNodetoTree(QTreeWidgetItem *father,
                                       const QString &filePath,
                                       utils::NodeType type);

  void traverseDir(QTreeWidgetItem *father, const QString &curPath);

private slots:

  // 添加文件目录节点
  void do_actFile_addNode_triggered(utils::NodeType);

  // 修改视图树可视化属性
  void do_actView_triggered();

  // 控制多选
  void on_actEdit_enbaleMultiSelect_triggered();

  // 删除节点
  void on_actFile_deleteFileNode_triggered();

  // 显示 About 窗口
  void on_actTool_about_triggered();

  // 选择密钥文件路径
  void on_toolButton_selectPath_clicked();

  // 是否使用密钥文件进行解压
  void on_checkBox_useKeytoDecompress_clicked();

  // 压缩操作按钮
  void on_pushButton_compress_clicked();

  // 解压操作按钮
  void on_pushButton_decompress_clicked();

private:
  Ui::MainWindow *ui;
};
