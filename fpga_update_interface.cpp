/*
 * @Author       : kydin hikydin@gmail.com
 * @Date         : 2025-11-01 05:41:09
 * @LastEditors  : kydin hikydin@gmail.com
 * @LastEditTime : 2025-11-05 21:48:15
 * @FilePath     : \QGowin\fpga_update_interface.cpp
 * @Description  :
 * Copyright (c) 2025 by kydin, email: hikydin@gmail.com, All
 * Rights Reserved.
 */
#include "fpga_update_interface.h"

#include <QRegularExpression>

FpgaUpdateInterface::FpgaUpdateInterface(QObject *parent) : QObject(parent) {
  process_ = new QProcess(this);
  connect(process_, &QProcess::readyReadStandardOutput, this,
          &FpgaUpdateInterface::HandleStdOutput);
  connect(process_, &QProcess::readyReadStandardError, this,
          &FpgaUpdateInterface::HandleStdError);
  connect(process_,
          QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
          &FpgaUpdateInterface::OnProcessFinished);
  connect(process_, &QProcess::errorOccurred, this,
          &FpgaUpdateInterface::OnProcessError);
}

FpgaUpdateInterface::~FpgaUpdateInterface() {}

void FpgaUpdateInterface::HandleStdOutput() {
  switch (action_) {
  case Fpga_Proces_Action_InProgress: {
    QByteArray data = process_->readAllStandardOutput();
    QString output = QString::fromUtf8(data);
    ParseFirmwareUpdateInfo(output); // 解析输出获取进度
    break;
  }
  default:
    break;
  }
}

void FpgaUpdateInterface::HandleStdError() {
  QByteArray data = process_->readAllStandardError();
  QString error = QString::fromUtf8(data);
  emit DebugMessage("Error Output:" + error);
}

void FpgaUpdateInterface::OnProcessFinished(int exitCode,
                                            QProcess::ExitStatus exitStatus) {
  switch (action_) {
  case Fpga_Proces_Action_ScanProbes: {
    probe_list_.clear();
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
      QByteArray output = process_->readAllStandardOutput();
      QString outputStr = QString::fromLocal8Bit(output);
      ParseProbeList(outputStr);
    } else {
      QByteArray standard_output = process_->readAllStandardOutput();
      QByteArray error_output = process_->readAllStandardError();
      emit DebugMessage("Action ScanDevice Output: " +
                        QString::fromLocal8Bit(standard_output));
      emit DebugMessage("Action ScanDevice Error: " +
                        QString::fromLocal8Bit(error_output));
    }
    emit DebugMessage("Scan probes finished");
    emit UpdateProbes(probe_list_);
    action_ = Fpga_Proces_Action_Idle;
    break;
  }
  case Fpga_Proces_Action_ScanDevice: {
    device_list_.clear();
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
      QByteArray output = process_->readAllStandardOutput();
      QString outputStr = QString::fromLocal8Bit(output);
      ParseDeviceList(outputStr);
    } else {
      QByteArray standard_output = process_->readAllStandardOutput();
      QByteArray error_output = process_->readAllStandardError();
      emit DebugMessage("Action ScanDevice Output: " +
                        QString::fromLocal8Bit(standard_output));
      emit DebugMessage("Action ScanDevice Error: " +
                        QString::fromLocal8Bit(error_output));
    }
    emit DebugMessage("Scan devices finished");
    emit UpdateDevices(device_list_);
    action_ = Fpga_Proces_Action_Idle;
    break;
  }
  case Fpga_Proces_Action_InProgress: {
    bool is_success = false;
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
      QByteArray output = process_->readAllStandardOutput();
      QString outputStr = QString::fromLocal8Bit(output);
      is_success = true;
    } else {
      QByteArray errorOutput = process_->readAllStandardError();
      emit DebugMessage(QString::fromLocal8Bit(errorOutput));
      is_success = false;
    }
    emit FirmwareUpdateFinished(is_success);
    emit DebugMessage("Firmware update finished");
    action_ = Fpga_Proces_Action_Idle;
    break;
  }
  }
}

void FpgaUpdateInterface::OnProcessError(QProcess::ProcessError error) {
  QString errorMsg;
  switch (error) {
  case QProcess::FailedToStart:
    errorMsg = "无法启动 gowin/programmer_cli.exe 命令，请确保 "
               "gowin/programmer_cli.exe 已安装并在 PATH 中";
    break;
  case QProcess::Crashed:
    errorMsg = "gowin/programmer_cli.exe 进程意外退出";
    break;
  default:
    errorMsg = "执行 gowin/programmer_cli.exe 命令时发生未知错误";
    break;
  }

  emit DebugMessage(errorMsg);
}

QVector<QString> &FpgaUpdateInterface::GetAllProbe() { return probe_list_; }

QVector<QString> &FpgaUpdateInterface::GetAllDevice() { return device_list_; }

void FpgaUpdateInterface::ScanDevice(QString gowin_cli_path) {
  emit DebugMessage("Scanning device...");
  action_ = Fpga_Proces_Action_ScanDevice;
  process_->start(gowin_cli_path, QStringList() << "--scan");
}

void FpgaUpdateInterface::ScanProbe(QString gowin_cli_path) {
  emit DebugMessage("Scanning probes...");
  action_ = Fpga_Proces_Action_ScanProbes;
  process_->start(gowin_cli_path, QStringList() << "--scan-cables");
}

void FpgaUpdateInterface::ParseProbeList(const QString &raw_output) {
  /*
  PS C:> .\programmer_cli.exe --scan-cables
  Cable found:  Gowin USB Cable(FT2CH)/0/786/null (USB location:786)
  Cost 0.05 second(s)
  */
  for (const QString &line : raw_output.split("\r\n")) {
    if (line.contains("Cable found:")) {
      QString probe = line.mid(QString("Cable found:").length() + 1).trimmed();
      probe_list_.append(probe);
    }
  }
}

void FpgaUpdateInterface::ParseDeviceList(const QString &raw_output) {
  /*
  失败
  PS C:> .\programmer_cli.exe --scan
  Scanning!
  Target Cable: Gowin USB Cable(FT2CH)/0/0/null@2.5MHz

  Error: No Gowin devices found!
  Cost 0.54 second(s)

  成功
  PS C:> .\programmer_cli.exe --scan
  Scanning!
  Target Cable: Gowin USB Cable(FT2CH)/0/0/null@2.5MHz
  Device Info:
      Family: GW1NRF
      Name: GW1N-4D GW1NR-4D GW1N-4B GW1NR-4B GW1NRF-4B  (One of them)
      ID: 0x1100381B
  1 device(s) found!
  Cost 0.54 second(s)
  */
  for (const QString &line : raw_output.split('\n')) {
    if (line.contains("ID:")) {
      QString id = line.mid(QString("ID:").length() + 1).trimmed();
      device_list_.append(id);
    }
  }
}

void FpgaUpdateInterface::StartFirmwareUpdate(FpgaUpdateParam param) {
  emit DebugMessage("Starting Firmware Update");
  QStringList arguments;
  arguments << "--operation_index" << QString::number(param.operation_index);
  if (!param.target_chip.isEmpty()) {
    arguments << "--device" << param.target_chip;
  }
  if (!param.firmware_path.isEmpty()) {
    arguments << "--fsFile" << param.firmware_path;
  } else {
    emit DebugMessage("Firmware path is empty!");
    return;
  }

  qDebug() << "QProcess run command: [" << param.gowin_cli_path << " "
           << arguments << "]";
  action_ = Fpga_Proces_Action_InProgress;
  process_->setProcessChannelMode(QProcess::MergedChannels); // 设置进程通道模式
  process_->start(param.gowin_cli_path, arguments);
}

void FpgaUpdateInterface::ParseFirmwareUpdateInfo(const QString &output) {
  // 升级阶段
  // gowin/programmer_cli.exe 通常输出类似 "\rProgramming...: [# ] 4% "
  // 的进度信息 校验阶段 gowin/programmer_cli.exe 通常输出类似 "\rVerifying...:
  // [#                        ] 4%                 " 的进度信息

  QRegularExpression progressRegex(
      R"(\r(Programming|Verifying)\s*\.{0,3}:\s*\[(#+)\s*\]\s*(\d+)%\s*)");
  QRegularExpressionMatch match = progressRegex.match(output);

  if (match.hasMatch()) {
    QString stage = match.captured(1);          // Programming 或 Verifying
    int percentage = match.captured(3).toInt(); // 百分比

    // 根据阶段处理进度
    if (stage == "Programming") {
      emit ProgrammingUpdated(percentage);
    } else if (stage == "Verifying") {
      emit VerifyingUpdated(percentage);
    }
  }
}
