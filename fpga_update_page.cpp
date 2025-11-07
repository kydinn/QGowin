/*
 * @Author       : kydin hikydin@gmail.com
 * @Date         : 2025-11-01 05:56:36
 * @LastEditors  : kydin hikydin@gmail.com
 * @LastEditTime : 2025-11-05 22:32:49
 * @FilePath     : \QGowin\fpga_update_page.cpp
 * @Description  :
 * Copyright (c) 2025 by kydin, email: hikydin@gmail.com, All
 * Rights Reserved.
 */
#include "fpga_update_page.h"

#include <QButtonGroup>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

static bool IsFileExist(QString fullFilePath) {
  QFileInfo fileInfo(fullFilePath);
  if (fileInfo.exists()) {
    return true;
  }
  return false;
}

static QString GetDateTime() {
  QDateTime time = QDateTime::currentDateTime();
  QString time_str = time.toString("yyyy-MM-dd hh:mm:ss");
  return "[" + time_str + "] ";
}

FpgaUpdatePage::FpgaUpdatePage(QWidget *parent)
    : QWidget(parent), ui(new Ui::FpgaUpdatePageClass()) {
  ui->setupUi(this);
  this->setWindowTitle(tr("固件升级"));

  // 初始化接口对象
  interface_ = new FpgaUpdateInterface(this);

  // 初始化页面
  for (auto it = device_map.constBegin(); it != device_map.constEnd(); it++) {
    ui->cb_target_chip->addItem(it.key());
  }
  for (auto it = flashing_method_map.constBegin();
       it != flashing_method_map.constEnd(); it++) {
    ui->cb_flashing_mode->addItem(it.key());
  }
  ui->cb_target_chip->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  ui->cb_flashing_mode->setSizePolicy(QSizePolicy::Expanding,
                                      QSizePolicy::Fixed);
  ui->cb_probes->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  ui->cb_devices->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  ui->cb_flashing_mode->setCurrentIndex(
      7); // 默认是 embFlash Erase,Program,Verify 模式
  ui->te_log->setReadOnly(true);
  ui->le_firmware_path->setReadOnly(true);
  ui->lbl_programming->hide();
  ui->progressbar_programming->hide();
  ui->progressbar_programming->setValue(0);
  ui->lbl_verifying->hide();
  ui->progressbar_verifying->hide();
  ui->progressbar_verifying->setValue(0);

  // 如果文件不存在则禁用页面
  if (!CheckFile()) {
    ui->btn_scan_probes->setDisabled(true);
    ui->btn_select_fiirmware->setDisabled(true);
    ui->btn_wirte_firmware->setDisabled(true);
  }

  // 连接信号与槽
  connect(ui->btn_scan_devices, &QPushButton::clicked, this,
          &FpgaUpdatePage::OnScanDevicesClicked);
  connect(ui->btn_scan_probes, &QPushButton::clicked, this,
          &FpgaUpdatePage::OnScanProbesClicked);
  connect(ui->btn_select_fiirmware, &QPushButton::clicked, this,
          &FpgaUpdatePage::OnSelectFiirmwareClicked);
  connect(ui->btn_wirte_firmware, &QPushButton::clicked, this,
          &FpgaUpdatePage::OnFirmwareUpdateClicked);
  connect(ui->cb_target_chip, &QComboBox::currentIndexChanged, this,
          &FpgaUpdatePage::CheckCanWriteFirmware);
  connect(ui->cb_flashing_mode, &QComboBox::currentIndexChanged, this,
          &FpgaUpdatePage::CheckCanWriteFirmware);
  connect(ui->cb_probes, &QComboBox::currentIndexChanged, this,
          &FpgaUpdatePage::CheckCanWriteFirmware);
  connect(ui->cb_devices, &QComboBox::currentIndexChanged, this,
          &FpgaUpdatePage::CheckCanWriteFirmware);
  connect(ui->le_firmware_path, &QLineEdit::textChanged, this,
          &FpgaUpdatePage::CheckCanWriteFirmware);
  connect(interface_, &FpgaUpdateInterface::UpdateProbes, this,
          &FpgaUpdatePage::UpdateProbes);
  connect(interface_, &FpgaUpdateInterface::UpdateDevices, this,
          &FpgaUpdatePage::UpdateDevices);
  connect(interface_, &FpgaUpdateInterface::DebugMessage, this,
          &FpgaUpdatePage::DebugMessage);
  connect(interface_, &FpgaUpdateInterface::ProgrammingUpdated, this,
          &FpgaUpdatePage::ProgrammingUpdated);
  connect(interface_, &FpgaUpdateInterface::VerifyingUpdated, this,
          &FpgaUpdatePage::VerifyingUpdated);
  connect(interface_, &FpgaUpdateInterface::FirmwareUpdateFinished, this,
          &FpgaUpdatePage::FirmwareUpdateFinished);

  // 页面初始化
  OnScanProbesClicked();
  CheckCanWriteFirmware();
}

FpgaUpdatePage::~FpgaUpdatePage() { delete ui; }

void FpgaUpdatePage::OnScanDevicesClicked() {
  LoadingPage();
  interface_->ScanDevice();
}

void FpgaUpdatePage::OnScanProbesClicked() {
  LoadingPage();
  interface_->ScanProbe();
}

void FpgaUpdatePage::OnSelectFiirmwareClicked() {
  QFileDialog *dialog = new QFileDialog(this);
  dialog->setWindowTitle(tr("选择固件文件"));
  dialog->setAcceptMode(QFileDialog::AcceptOpen);
  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setViewMode(QFileDialog::Detail);

  QStringList filters;
  filters << "数据流文件 (*.fs)";
  dialog->setNameFilters(filters);
  dialog->selectNameFilter(filters[0]);
  dialog->setDirectory(QDir::homePath());

  if (dialog->exec() == QDialog::Accepted) {
    QString file_path = dialog->selectedFiles()[0];
    if (!file_path.isEmpty() && IsFileExist(file_path)) {
      QString filename = QFileInfo(file_path).fileName();
      ui->le_firmware_path->setText(file_path);
    }
  }
}

void FpgaUpdatePage::OnFirmwareUpdateClicked() {
  FpgaUpdateParam param;
  QString target_chip = ui->cb_target_chip->currentText();
  QString flashing_mode = ui->cb_flashing_mode->currentText();
  QString firmware_path = ui->le_firmware_path->text();

  if (!IsFileExist(firmware_path)) {
    QMessageBox::critical(this, tr("错误"), tr("固件文件不存在"));
    return;
  }

  for (auto it = flashing_method_map.constBegin();
       it != flashing_method_map.constEnd(); it++) {
    if (it.key() == ui->cb_flashing_mode->currentText()) {
      param.operation_index = it.value();
    }
  }
  param.gowin_cli_path = DEFAULT_GOWIN_CLI_PATH;
  param.target_chip = target_chip;
  param.firmware_path = firmware_path;

  LoadingPage();
  ui->lbl_programming->show();
  ui->progressbar_programming->setValue(0);
  ui->progressbar_programming->show();
  ui->lbl_verifying->show();
  ui->progressbar_verifying->setValue(0);
  ui->progressbar_verifying->show();
  interface_->StartFirmwareUpdate(param);
}

void FpgaUpdatePage::UpdateProbes(const QVector<QString> &probes) {
  UnLoadingPage();
  ui->cb_probes->clear();
  for (int i = 0; i < probes.size(); i++) {
    ui->cb_probes->addItem(probes[i]);
  }
}

void FpgaUpdatePage::UpdateDevices(const QVector<QString> &devices) {
  UnLoadingPage();
  ui->cb_devices->clear();
  for (int i = 0; i < devices.size(); i++) {
    ui->cb_devices->addItem(devices[i]);
  }
}

void FpgaUpdatePage::DebugMessage(const QString &msg) {
  ui->te_log->append(GetDateTime() + msg);
}

void FpgaUpdatePage::ProgrammingUpdated(int percentage) {
  ui->progressbar_programming->setValue(percentage);
}

void FpgaUpdatePage::VerifyingUpdated(int percentage) {
  ui->progressbar_verifying->setValue(percentage);
}

void FpgaUpdatePage::FirmwareUpdateFinished(bool success) {
  if (success) {
    QMessageBox::information(this, tr("提示"), tr("固件升级完成"));
  } else {
    QMessageBox::critical(this, tr("错误"), tr("固件升级失败"));
  }
  UnLoadingPage();
  ui->lbl_programming->hide();
  ui->progressbar_programming->hide();
  ui->lbl_verifying->hide();
  ui->progressbar_verifying->hide();
}

void FpgaUpdatePage::LoadingPage() {
  ui->btn_select_fiirmware->setDisabled(true);
}

void FpgaUpdatePage::UnLoadingPage() {
  ui->btn_select_fiirmware->setDisabled(false);
}

void FpgaUpdatePage::CheckCanWriteFirmware() {
  bool is_valid = false;
  is_valid = (ui->cb_target_chip->count() > 0) ? true : false;
  is_valid = is_valid && (ui->cb_flashing_mode->count() > 0) ? true : false;
  is_valid = is_valid && (ui->cb_probes->count() > 0) ? true : false;
  is_valid = is_valid && (ui->cb_devices->count() > 0) ? true : false;
  is_valid = is_valid && !ui->le_firmware_path->text().isEmpty();
  ui->btn_wirte_firmware->setEnabled(is_valid);
}

bool FpgaUpdatePage::CheckFile() {
  QString app_path = QCoreApplication::applicationDirPath();

  // check gowin
  QString gowin_cli_path = DEFAULT_GOWIN_DIR_PATH + "programmer_cli.exe";
  if (!IsFileExist(gowin_cli_path)) {
    QMessageBox::critical(this, tr("错误"),
                          tr("缺少必要文件") + gowin_cli_path);
    return false;
  }
  return true;
}
