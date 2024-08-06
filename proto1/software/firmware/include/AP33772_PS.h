/**
 * @file AP33772_PS.h
 * @brief AP33772_PS Class Header File
 * @details AP33772_PS is the class to define the interfaces for power supply usage
 */

#ifndef __AP33772_PS__
#define __AP33772_PS__

#include <vector>

#include "AP33772.h"

typedef enum
{
  AP33772_PS_OK,
  AP33772_PS_NG,
  AP33772_PS_PARAM_NG,
  AP33772_PS_INIT_NG,
  AP33772_PS_MODE_NG,
} AP33772_PS_ER;

typedef enum
{
  AP33772_PS_MODE_FIXED,
  AP33772_PS_MODE_ADJ,
} AP33772_PS_MODE;

class AP33772_PS
{
public:
  AP33772_PS(TwoWire &wire);
  ~AP33772_PS();

  AP33772_PS_ER setup(pin_size_t sdaPin, pin_size_t sclPin);
  AP33772_PS_ER begin();

  AP33772_PS_ER setMode(AP33772_PS_MODE mode);
  AP33772_PS_ER getMode(AP33772_PS_MODE &mode);

  AP33772_PS_ER setOutput(bool enable);
  AP33772_PS_ER getOutput(bool &enable);

  AP33772_PS_ER setOperatingCurrent(int current);
  AP33772_PS_ER getOperatingCurrent(int &current);

  AP33772_PS_ER setVoltage(int voltage);
  AP33772_PS_ER getVoltage(int &voltage);

  AP33772_PS_ER getNumOfFixedVoltages(size_t &num);
  AP33772_PS_ER getFixedVoltageValue(size_t index, int &voltage);

  AP33772_PS_ER getCurrent(int &current);

  AP33772* getRawCtrl();


private:
  AP33772_PS_ER updateControl();

  bool m_isInit;
  TwoWire* m_wire;
  AP33772 *m_ap33772;
  AP33772_PS_MODE m_mode;
  bool m_outputEnable;
  int m_voltage;
  int m_operatingCurrent;
  int m_baseCurrent;

  std::vector<int> m_fixedVoltages;
};

#endif
