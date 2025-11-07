/*
 * @Author       : kydin hikydin@gmail.com
 * @Date         : 2025-11-01 05:56:36
 * @LastEditors  : kydin hikydin@gmail.com
 * @LastEditTime : 2025-11-05 22:31:31
 * @FilePath     : \QGowin\fpga_update_page.h
 * @Description  :
 * Copyright (c) 2025 by kydin, email: hikydin@gmail.com, All
 * Rights Reserved.
 */
#pragma once

#include "ui_fpga_update_page.h"
#include <QWidget>

#include "fpga_update_interface.h"

static QMap<QString, QString> device_map = {
    {"GW1N-4D", "GW1N-4D"},
};

static QMap<QString, int> flashing_method_map = {
    {"Read Device Codes", 0},
    {"Reprogram", 1},
    {"SRAM Program", 2},
    {"SRAM Read", 3},
    {"SRAM Program and Verify", 4},
    {"embFlash Erase,Program", 5},
    {"embFlash Erase,Program,Verify", 6},
    {"embFlash Erase Only", 7},
    {"exFlash Erase,Program", 8},
    {"exFlash Erase,Program,Verify", 9},
};

QT_BEGIN_NAMESPACE
namespace Ui {
class FpgaUpdatePageClass;
};
QT_END_NAMESPACE

class FpgaUpdatePage : public QWidget {
  Q_OBJECT

public:
  FpgaUpdatePage(QWidget *parent = nullptr);
  ~FpgaUpdatePage();

public slots:
  void OnScanDevicesClicked();
  void OnScanProbesClicked();
  void OnSelectFiirmwareClicked();
  void OnFirmwareUpdateClicked();
  void UpdateProbes(const QVector<QString> &probes);
  void UpdateDevices(const QVector<QString> &devices);
  void DebugMessage(const QString &msg);
  void ProgrammingUpdated(int percentage);
  void VerifyingUpdated(int percentage);
  void FirmwareUpdateFinished(bool success);
  void LoadingPage();
  void UnLoadingPage();
  void CheckCanWriteFirmware();

private:
  bool CheckFile();

private:
  Ui::FpgaUpdatePageClass *ui;
  FpgaUpdateInterface *interface_;
};
