/*
 * @Author       : kydin hikydin@gmail.com
 * @Date         : 2025-11-01 05:41:09
 * @LastEditors  : kydin hikydin@gmail.com
 * @LastEditTime : 2025-11-04 22:51:52
 * @FilePath     : \QGowin\fpga_update_interface.h
 * @Description  :
 * Copyright (c) 2025 by kydin, email: hikydin@gmail.com, All
 * Rights Reserved.
 */
#pragma once

#include <QCoreApplication>
#include <QObject>
#include <QProcess>

#define DEFAULT_GOWIN_DIR_PATH                                                 \
  (QCoreApplication::applicationDirPath() + "/tools/gowin/")
#define DEFAULT_GOWIN_CLI_PATH (DEFAULT_GOWIN_DIR_PATH + "programmer_cli.exe")

typedef struct _FpgaUpdateParam {
  QString gowin_cli_path;
  int operation_index;
  QString target_chip;
  QString firmware_path;
} FpgaUpdateParam;

typedef enum _FpgaProcessActions {
  Fpga_Proces_Action_Idle,
  Fpga_Proces_Action_ScanDevice,
  Fpga_Proces_Action_ScanProbes,
  Fpga_Proces_Action_InProgress,
} FpgaProcessActions;

class FpgaUpdateInterface : public QObject {
  Q_OBJECT

public:
  FpgaUpdateInterface(QObject *parent);
  ~FpgaUpdateInterface();

  void HandleStdOutput();
  void HandleStdError();
  void OnProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void OnProcessError(QProcess::ProcessError error);
  QVector<QString> &GetAllProbe();
  QVector<QString> &GetAllDevice();
  void ScanDevice(QString gowin_cli_path = DEFAULT_GOWIN_CLI_PATH);
  void ScanProbe(QString gowin_cli_path = DEFAULT_GOWIN_CLI_PATH);
  void ParseProbeList(const QString &raw_output);
  void ParseDeviceList(const QString &raw_output);
  void StartFirmwareUpdate(FpgaUpdateParam info);
  void ParseFirmwareUpdateInfo(const QString &output);

signals:
  void ProbesReady(const QStringList &probes);
  void ErrorOccurred(const QString &errorMessage);
  void ProgrammingUpdated(int percentage);
  void VerifyingUpdated(int percentage);
  void StatusUpdated(const QString &message);
  void FirmwareUpdateFinished(bool success);
  void UpdateProbes(const QVector<QString> &);
  void UpdateDevices(const QVector<QString> &);
  void DebugMessage(const QString &msg);

private:
  FpgaProcessActions action_;
  QProcess *process_;
  QVector<QString> probe_list_;
  QVector<QString> device_list_;
};
