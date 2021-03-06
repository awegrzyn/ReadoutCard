/// \file RocPciDevice.cxx
/// \brief Implementation of the RocPciDevice class.
///
/// \author Pascal Boeschoten (pascal.boeschoten@cern.ch)
/// \author Kostas Alexopoulos (kostas.alexopoulos@cern.ch)

#include "RocPciDevice.h"

#include <map>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <functional>
#include <iostream>
#include "Crorc/Crorc.h"
#include "Cru/CruBar.h"
#include "Pda/PdaBar.h"
#include "Pda/PdaDevice.h"
#include "ReadoutCard/ChannelFactory.h"
#include "ReadoutCard/Exception.h"
#include "ReadoutCard/Parameters.h"
#include "Utilities/SmartPointer.h"

namespace AliceO2 {
namespace roc {

boost::optional<int32_t> crorcGetSerial(Pda::PdaDevice::PdaPciDevice pciDevice);
boost::optional<int32_t> cruGetSerial(Pda::PdaDevice::PdaPciDevice pciDevice);

namespace {
struct DeviceType
{
    CardType::type cardType;
    const PciId pciId;
    std::function<boost::optional<int32_t> (Pda::PdaDevice::PdaPciDevice pciDevice)> getSerial;
};

const std::vector<DeviceType> deviceTypes = {
    { CardType::Crorc, {"0033", "10dc"}, crorcGetSerial }, // C-RORC
    { CardType::Cru, {"e001", "1172"}, cruGetSerial }, // Altera dev board CRU
};

PciAddress addressFromDevice(Pda::PdaDevice::PdaPciDevice pciDevice){
  uint8_t busId;
  uint8_t deviceId;
  uint8_t functionId;
  if (PciDevice_getBusID(pciDevice.get(), &busId) || PciDevice_getDeviceID(pciDevice.get(), &deviceId)
      || PciDevice_getFunctionID(pciDevice.get(), &functionId)) {
    BOOST_THROW_EXCEPTION(Exception() << ErrorInfo::Message("Failed to retrieve device address"));
  }
  PciAddress address (busId, deviceId, functionId);
  return address;
}

CardDescriptor defaultDescriptor() {
  return {CardType::Unknown, -1, {"unknown", "unknown"}, PciAddress(0,0,0), -1};
}
} // Anonymous namespace

void RocPciDevice::initWithSerial(int serialNumber)
{
  try {
    for (const auto& type : deviceTypes) {
      mPdaDevice = Pda::PdaDevice::getPdaDevice(type.pciId);
      for (auto& pciDevice : mPdaDevice->getPciDevices(mPdaDevice)) {
        if (type.getSerial(pciDevice) == serialNumber) {
          Utilities::resetSmartPtr(mPciDevice, pciDevice);
          mDescriptor = CardDescriptor{type.cardType, serialNumber, type.pciId, addressFromDevice(pciDevice), PciDevice_getNumaNode(pciDevice.get())};
          return;
        }
      }
    }
    BOOST_THROW_EXCEPTION(Exception() << ErrorInfo::Message("Could not find card"));
  }
  catch (boost::exception& e) {
    e << ErrorInfo::SerialNumber(serialNumber);
    addPossibleCauses(e, {"Invalid serial number search target"});
    throw;
  }
}

void RocPciDevice::initWithAddress(const PciAddress& address)
{
  try {
    for (const auto& type : deviceTypes) {
      mPdaDevice = Pda::PdaDevice::getPdaDevice(type.pciId);
      for (const auto& pciDevice : mPdaDevice->getPciDevices(mPdaDevice)) {
        if (addressFromDevice(pciDevice) == address) {
          Utilities::resetSmartPtr(mPciDevice, pciDevice);
          mDescriptor = CardDescriptor { type.cardType, type.getSerial(pciDevice), type.pciId, address, PciDevice_getNumaNode(pciDevice.get())};
          return;
        }
      }
    }
    BOOST_THROW_EXCEPTION(Exception() << ErrorInfo::Message("Could not find card"));
  } catch (boost::exception& e) {
    e << ErrorInfo::PciAddress(address);
    addPossibleCauses(e, { "Invalid PCI address search target" });
    throw;
  }
}

RocPciDevice::RocPciDevice(int serialNumber)
    : mDescriptor(defaultDescriptor())
{
  initWithSerial(serialNumber);
}

RocPciDevice::RocPciDevice(const PciAddress& address)
    : mDescriptor(defaultDescriptor())
{
  initWithAddress(address);
}

RocPciDevice::RocPciDevice(const Parameters::CardIdType& cardId)
    : mDescriptor(defaultDescriptor())
{
  if (auto serial = boost::get<int>(&cardId)) {
    initWithSerial(*serial);
  } else {
    initWithAddress(boost::get<PciAddress>(cardId));
  }
}

RocPciDevice::~RocPciDevice()
{
}

std::vector<CardDescriptor> RocPciDevice::findSystemDevices()
{
  std::vector<CardDescriptor> cards;
  for (const auto& type : deviceTypes) {
    for (const auto& pciDevice : Pda::PdaDevice::getPciDevices(type.pciId)) {
      cards.push_back(CardDescriptor{type.cardType, type.getSerial(pciDevice), type.pciId,
        addressFromDevice(pciDevice), PciDevice_getNumaNode(pciDevice.get())});
    }
  }
  return cards;
}

std::vector<CardDescriptor> RocPciDevice::findSystemDevices(int serialNumber)
{
  std::vector<CardDescriptor> cards;
  try {
    for (const auto& type : deviceTypes) {
      for (const auto& pciDevice : Pda::PdaDevice::getPciDevices(type.pciId)) {
        if (type.getSerial(pciDevice) == serialNumber) {
          cards.push_back(CardDescriptor{type.cardType, type.getSerial(pciDevice), type.pciId,
                  addressFromDevice(pciDevice), PciDevice_getNumaNode(pciDevice.get())});
        }
      }
    }
  }
  catch (boost::exception& e) {
    e << ErrorInfo::SerialNumber(serialNumber);
    addPossibleCauses(e, {"Invalid serial number search target"});
    throw;
  }
  return cards;
}

std::vector<CardDescriptor> RocPciDevice::findSystemDevices(const PciAddress& address)
{
  std::vector<CardDescriptor> cards;
  try {
    for (const auto& type : deviceTypes) {
      for (const auto& pciDevice : Pda::PdaDevice::getPciDevices(type.pciId)) {
        if (addressFromDevice(pciDevice) == address) {
          cards.push_back(CardDescriptor{type.cardType, type.getSerial(pciDevice), type.pciId, address, PciDevice_getNumaNode(pciDevice.get())});
        }
      }
    }
  }
  catch (boost::exception& e) {
    e << ErrorInfo::PciAddress(address);
    addPossibleCauses(e, {"Invalid PCI address search target"});
    throw;
  }
  return cards;
}

void RocPciDevice::printDeviceInfo(std::ostream& ostream)
{
  uint16_t domainId;
  uint8_t busId;
  uint8_t functionId;
  const PciBarTypes* pciBarTypesPtr;

  if (PciDevice_getDomainID(mPciDevice->get(), &domainId) || PciDevice_getBusID(mPciDevice->get(), &busId)
      || PciDevice_getFunctionID(mPciDevice->get(), &functionId)
      || PciDevice_getBarTypes(mPciDevice->get(), &pciBarTypesPtr)) {
    BOOST_THROW_EXCEPTION(Exception() << ErrorInfo::Message("Failed to retrieve device info"));
  }

  auto barType = *pciBarTypesPtr;
  auto barTypeString =
      barType == PCIBARTYPES_NOT_MAPPED ? "NOT_MAPPED" :
      barType == PCIBARTYPES_IO ? "IO" :
      barType == PCIBARTYPES_BAR32 ? "BAR32" :
      barType == PCIBARTYPES_BAR64 ? "BAR64" :
      "n/a";

  auto f = boost::format("%-14s %10s\n");
  ostream << f % "Domain ID" << domainId;
  ostream << f % "Bus ID" << busId;
  ostream << f % "Function ID" << functionId;
  ostream << f % "BAR type" << barTypeString;
}

boost::optional<int32_t> cruGetSerial(Pda::PdaDevice::PdaPciDevice pciDevice)
{
  std::shared_ptr<Pda::PdaBar> pdaBar2;
  Utilities::resetSmartPtr(pdaBar2, pciDevice, 2);
  return CruBar(pdaBar2).getSerial();
}

boost::optional<int32_t> crorcGetSerial(Pda::PdaDevice::PdaPciDevice pciDevice)
{
  Pda::PdaBar pdaBar(pciDevice, 0); // Must use BAR 0 to access flash
  return Crorc::getSerial(pdaBar);
}

} // namespace roc
} // namespace AliceO2
