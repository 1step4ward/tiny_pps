/**
 * @file AP33772_PS.cpp
 * @brief AP33772_PS Class 
 * @details AP33772_PS is the class to define the interfaces for power supply usage
 */

#include "AP33772_PS.h"
#include "pin_config.h"


/**
 * @brief Class constuctor
 * @param &wire reference of Wire class. Pass in Wire or Wire1
 */
AP33772_PS::AP33772_PS(TwoWire &wire)
{
  m_isInit = false;
  m_mode = AP33772_PS_MODE_ADJ;
  m_outputEnable = false;
  m_voltage = 0;
  m_operatingCurrent = 0;
  m_baseCurrent = 0;

  m_wire = &wire;
  m_ap33772 = new AP33772(wire);
}

/**
 * @brief Class destructor
 */
AP33772_PS::~AP33772_PS()
{
  delete m_ap33772;
}

/**
 * @brief Setup function
 */
AP33772_PS_ER AP33772_PS::setup(pin_size_t sdaPin, pin_size_t sclPin)
{
  m_wire->setSDA(sdaPin);
  m_wire->setSCL(sclPin);

  return AP33772_PS_OK;
}

/**
 * @brief Initialize function
 */
AP33772_PS_ER AP33772_PS::begin()
{
  m_wire->begin();

  while (m_ap33772->getNumPDO() == 0) {
    m_ap33772->begin();
  }

  /* set fixed voltages */
  byte numPDO = m_ap33772->getNumPDO();
  PDO_DATA_T* pdoData = m_ap33772->getPdoData();
  for (byte i = 0; i < numPDO; i++)
  {
    if ((pdoData[i].byte3 & 0xC0) == 0x00) // Fixed PDO
    {
      m_fixedVoltages.push_back(pdoData[i].fixed.voltage * 50);
    }
  }

  m_baseCurrent = getRawCtrl()->readCurrent();
  m_isInit = true;
  return AP33772_PS_OK;
}

/**
 * @brief Set mode function
 * @param [in] mode Fixed or Adjustable 
 */
AP33772_PS_ER AP33772_PS::setMode(AP33772_PS_MODE mode)
{
  if (!m_isInit) return AP33772_PS_INIT_NG;

  m_mode = mode;
  m_outputEnable = false;

  updateControl();
  return AP33772_PS_OK;
}

/**
 * @brief Get mode function
 * @param [out] mode Fixed or Adjustable 
 */
AP33772_PS_ER AP33772_PS::getMode(AP33772_PS_MODE &mode)
{
  if (!m_isInit) return AP33772_PS_INIT_NG;

  mode = m_mode;

  return AP33772_PS_OK;
}

/**
 * @brief Set output function
 * @param [in] enable true: output enabled, false: output disabled
 */
AP33772_PS_ER AP33772_PS::setOutput(bool enable)
{
  if (!m_isInit) return AP33772_PS_INIT_NG;

  m_outputEnable = enable;

  updateControl();
  return AP33772_PS_OK;
}

/**
 * @brief Get output function
 * @param [out] enable true: output enabled, false: output disabled
 */
AP33772_PS_ER AP33772_PS::getOutput(bool &enable)
{
  if (!m_isInit) return AP33772_PS_INIT_NG;

  enable = m_outputEnable;

  return AP33772_PS_OK;
}

/**
 * @brief Set operating current function
 * @param [in] current current value [mA]
 */
AP33772_PS_ER AP33772_PS::setOperatingCurrent(int current)
{
  if (!m_isInit) return AP33772_PS_INIT_NG;

  m_operatingCurrent = current;

  updateControl();
  return AP33772_PS_OK;
}

/**
 * @brief Get operating current function
 * @param [out] current current value [mA]
 */
AP33772_PS_ER AP33772_PS::getOperatingCurrent(int &current)
{
  if (!m_isInit) return AP33772_PS_INIT_NG;

  current = m_operatingCurrent;

  return AP33772_PS_OK;
}

/**
 * @brief Set voltage function
 * @param [in] voltage voltage value [mV]
 */
AP33772_PS_ER AP33772_PS::setVoltage(int voltage)
{
  if (!m_isInit) return AP33772_PS_INIT_NG;

  m_voltage = voltage;

  updateControl();
  return AP33772_PS_OK;
}

/**
 * @brief Get voltage function
 * @param [out] voltage voltage value [mV]
 */
AP33772_PS_ER AP33772_PS::getVoltage(int &voltage)
{
  if (!m_isInit) return AP33772_PS_INIT_NG;

  voltage = m_voltage;

  return AP33772_PS_OK;
}

/**
 * @brief Get number of fixed voltages function
 * @param [out] num number of fixed voltages
 */
AP33772_PS_ER AP33772_PS::getNumOfFixedVoltages(size_t &num)
{
  if (!m_isInit) return AP33772_PS_INIT_NG;
  
  num = m_fixedVoltages.size();

  return AP33772_PS_OK;
}

/**
 * @brief Get fixed voltage from index function
 * @param [in] index index of fixed voltages
 * @param [out] voltage voltage value [mV]
 */
AP33772_PS_ER AP33772_PS::getFixedVoltageValue(size_t index, int &voltage)
{
  if (!m_isInit) return AP33772_PS_INIT_NG;

  size_t num = m_fixedVoltages.size();
  if (index >= num) return AP33772_PS_PARAM_NG;

  voltage = m_fixedVoltages[index];

  return AP33772_PS_OK;
}

/**
 * @brief Update actual control for AP33772
 */
AP33772_PS_ER AP33772_PS::updateControl()
{
  if (!m_isInit) return AP33772_PS_INIT_NG;

  // AP33772_PS_MODE m_mode;

  if (!m_outputEnable) {
    /* change first for disable case */
    digitalWrite(16, LOW);
    pinMode(16, INPUT_PULLDOWN); //MOSFET
  }

  m_ap33772->setMaxCurrent(m_operatingCurrent);
  m_ap33772->setVoltage(m_voltage);
  // m_ap33772->setVoltageCurrent(m_voltage, m_operatingCurrent);

  if (m_outputEnable) {
    /* change at last for disable case */
    pinMode(16, OUTPUT); //MOSFET
    digitalWrite(16, HIGH);
  }

  return AP33772_PS_OK;
}

/**
 * @brief Get operating current function
 * @param [out] current current value [mA]
 */
AP33772_PS_ER AP33772_PS::getCurrent(int &current)
{
  if (!m_isInit) return AP33772_PS_INIT_NG;

  current = getRawCtrl()->readCurrent() - m_baseCurrent;

  return AP33772_PS_OK;
}

/**
 * @brief Update actual control for AP33772
 */
AP33772* AP33772_PS::getRawCtrl()
{
  return m_ap33772;
}