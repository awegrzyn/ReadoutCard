/// \file CruBar.h
/// \brief Definition of the CruBar class.
///
/// \author Pascal Boeschoten (pascal.boeschoten@cern.ch)
/// \author Kostas Alexopoulos (kostas.alexopoulos@cern.ch)

#ifndef ALICEO2_READOUTCARD_CRU_CRUBAR_H_
#define ALICEO2_READOUTCARD_CRU_CRUBAR_H_

#include <cstddef>
#include <boost/optional/optional.hpp>
#include "BarInterfaceBase.h"
#include "Cru/Constants.h"
#include "Cru/FirmwareFeatures.h"
#include "ExceptionInternal.h"
#include "Pda/PdaBar.h"
#include "Utilities/Util.h"

namespace AliceO2 {
namespace roc {

class CruBar final : public BarInterfaceBase
{
  public:
    CruBar(const Parameters& parameters);
    CruBar(std::shared_ptr<Pda::PdaBar> bar);
    virtual ~CruBar();
    //virtual void checkReadSafe(int index) override;
    //virtual void checkWriteSafe(int index, uint32_t value) override;

    virtual CardType::type getCardType() override
    {
      return CardType::Cru;
    }

    virtual boost::optional<int32_t> getSerial() override;
    virtual boost::optional<float> getTemperature() override;
    virtual boost::optional<std::string> getFirmwareInfo() override;
    virtual boost::optional<std::string> getCardId() override;
    virtual int32_t getDroppedPackets() override;
    virtual uint32_t getCTPClock() override;
    virtual uint32_t getLocalClock() override;
    virtual int32_t getLinksPerWrapper(uint32_t wrapper) override;
    virtual int32_t getLinks() override;



    void pushSuperpageDescriptor(uint32_t link, uint32_t pages, uintptr_t busAddress);
    uint32_t getSuperpageCount(uint32_t link);
    void setDataEmulatorEnabled(bool enabled) const;
    void resetDataGeneratorCounter() const;
    void resetCard() const;
    void setDataGeneratorPattern(GeneratorPattern::type pattern, size_t size, bool randomEnabled);
    void dataGeneratorInjectError();
    void setDataSource(uint32_t source);
    void setLinksEnabled(uint32_t mask);
    FirmwareFeatures getFirmwareFeatures();
 
    static FirmwareFeatures convertToFirmwareFeatures(uint32_t reg);

    static void setDataGeneratorPatternBits(uint32_t& bits, GeneratorPattern::type pattern);

    static void setDataGeneratorSizeBits(uint32_t& bits, size_t size);
    static void setDataGeneratorEnableBits(uint32_t& bits, bool enabled);
    static void setDataGeneratorRandomSizeBits(uint32_t& bits, bool enabled);

  private:
    uint32_t getSerialNumber() const;
    uint32_t getTemperatureRaw() const;
    boost::optional<float> convertTemperatureRaw(uint32_t registerValue) const;
    boost::optional<float> getTemperatureCelsius() const;
    uint32_t getFirmwareCompileInfo();
    uint32_t getFirmwareGitHash();
    uint32_t getFirmwareDateEpoch();
    uint32_t getFirmwareDate();
    uint32_t getFirmwareTime();
    uint32_t getFpgaChipHigh();
    uint32_t getFpgaChipLow();

    FirmwareFeatures parseFirmwareFeatures();
 
    FirmwareFeatures mFeatures;

    /// Checks if this is the correct BAR. Used to check for BAR 2 for special functions.
    void assertBarIndex(int index, std::string message) const
    {
      if (mPdaBar->getIndex() != index) {
        BOOST_THROW_EXCEPTION(Exception() << ErrorInfo::Message(message) << ErrorInfo::BarIndex(mPdaBar->getIndex()));
      }
    }

};

} // namespace roc
} // namespace AliceO2

#endif // ALICEO2_READOUTCARD_CRU_CRUBAR_H_
