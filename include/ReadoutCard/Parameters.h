/// \file Parameters.h
/// \brief Definition of the RORC Parameters and associated functions
///
/// \author Pascal Boeschoten (pascal.boeschoten@cern.ch)

#ifndef ALICEO2_INCLUDE_READOUTCARD_PARAMETERS_H_
#define ALICEO2_INCLUDE_READOUTCARD_PARAMETERS_H_

#include <map>
#include <memory>
#include <set>
#include <string>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include "ReadoutCard/ParameterTypes/BufferParameters.h"
#include "ReadoutCard/ParameterTypes/GeneratorPattern.h"
#include "ReadoutCard/ParameterTypes/LoopbackMode.h"
#include "ReadoutCard/ParameterTypes/PciAddress.h"
#include "ReadoutCard/ParameterTypes/ReadoutMode.h"

namespace AliceO2 {
namespace roc {

struct ParametersPimpl;

/// Class that holds parameters for channels
/// Per parameter, it has three functions:
/// * Setter
/// * Non-throwing getter that returns the value wrapped in an optional if it is present, or an empty optional if it
///   was not
/// * Throwing getter that returns the value if it is present, or throws a ParameterException
class Parameters
{
  public:
    Parameters();
    ~Parameters();
    Parameters(const Parameters& other);
    Parameters(Parameters&& other);
    Parameters& operator=(const Parameters& other);
    Parameters& operator=(Parameters&& other);


    // Types for parameter values

    /// Type for buffer parameters. It can hold Memory, File or Null buffer parameters.
    using BufferParametersType = boost::variant<buffer_parameters::Memory, buffer_parameters::File,
        buffer_parameters::Null>;

    /// Type for the CardId parameter. It can hold either a serial number or PciAddress.
    using CardIdType = boost::variant<int, ::AliceO2::roc::PciAddress>;

    /// Type for the ChannelNumber parameter
    using ChannelNumberType = int32_t;

    /// Type for the DMA page size parameter
    using DmaPageSizeType = size_t;

    /// Type for the generator enabled parameter
    using GeneratorEnabledType = bool;

    /// Type for the generator data size parameter
    using GeneratorDataSizeType = size_t;

    /// Type for the LoopbackMode parameter
    using GeneratorLoopbackType = LoopbackMode::type;

    /// Type for the generator pattern parameter
    using GeneratorPatternType = GeneratorPattern::type;

    /// Type for the generator data size parameter
    using GeneratorRandomSizeEnabledType = bool;

    /// Type for the readout mode parameter
    using ReadoutModeType = ReadoutMode::type;

    /// Type for the link mask parameter
    using LinkMaskType = std::set<uint32_t>;


    // Setters

    /// Sets the CardId parameter
    ///
    /// This can be either a PciAddress, or an integer representing a serial number.
    /// It may be -1 to instantiate the dummy driver.
    ///
    /// Required parameter.
    ///
    /// \param value The value to set
    /// \return Reference to this object for chaining calls
    auto setCardId(CardIdType value) -> Parameters&;

    /// Sets the ChannelNumber parameter
    ///
    /// This indicates which DMA channel should be opened.
    /// * The C-RORC has 6 available channels (numbers 0 to 5).
    /// * The CRU has one (number 0).
    ///
    /// Required parameter.
    ///
    /// \param value The value to set
    /// \return Reference to this object for chaining calls
    auto setChannelNumber(ChannelNumberType value) -> Parameters&;

    /// Sets the DmaPageSize parameter
    ///
    /// Supported values:
    /// * C-RORC: ??? (it seems to be very flexible)
    /// * CRU: 8 KiB
    ///
    /// If not set, the card's driver will select a sensible default
    ///
    /// NOTE: Will probably be removed. In which case for the C-RORC this will be set per superpage to the superpage
    ///   size. For the CRU, it is non-configurable anyway.
    ///
    /// \param value The value to set
    /// \return Reference to this object for chaining calls
    auto setDmaPageSize(DmaPageSizeType value) -> Parameters&;

    /// Sets the GeneratorEnabled parameter
    ///
    /// If enabled, the card will generate data internally.
    /// The format and content of this data is controlled by the other generator parameters.
    /// 'None' loopback mode is not allowed in conjunction with the data generator (see setGeneratorLoopback()).
    ///
    /// \param value The value to set
    /// \return Reference to this object for chaining calls
    auto setGeneratorEnabled(GeneratorEnabledType value) -> Parameters&;

    /// Sets the GeneratorDataSize parameter
    ///
    /// It controls the size in bytes of the generated data per DMA page.
    ///
    /// Supported values:
    /// * C-RORC: multiples of 4 bytes, up to 2097152 bytes.
    /// * CRU: multiples of 32 bytes, minimum 64 bytes, up to 8 KiB.
    ///
    /// If not set, the driver will default to the DMA page size, i.e. the pages will be filled completely.
    ///
    /// \param value The value to set
    /// \return Reference to this object for chaining calls
    auto setGeneratorDataSize(GeneratorDataSizeType value) -> Parameters&;

    /// Sets the GeneratorLoopback parameter
    ///
    /// Controls the routing of the generated data.
    /// Supported loopback modes:
    /// * C-RORC: all modes
    /// * CRU: internal, none (=external)
    ///
    /// If not set, the driver will default to internal loopback.
    ///
    /// \param value The value to set
    /// \return Reference to this object for chaining calls
    auto setGeneratorLoopback(GeneratorLoopbackType value) -> Parameters&;

    /// Sets the GeneratorPattern parameter.
    ///
    /// Determines the content of the generated data.
    /// Supported formats:
    /// * C-RORC: all formats (I think...)
    /// * CRU: Constant, Alternating, Incremental
    ///
    /// If not set, the driver will default to the incremental pattern (GeneratorPattern::Incremental).
    ///
    /// \param value The value to set
    /// \return Reference to this object for chaining calls
    auto setGeneratorPattern(GeneratorPatternType value) -> Parameters&;

    /// Sets the GeneratorRandomSizeEnabled parameter.
    ///
    /// If enabled, the content of the DMA pages will have a random size.
    /// * C-RORC: currently unsupported
    /// * CRU: Size varies between 32 bytes and the DMA page size
    ///
    /// \param value The value to set
    /// \return Reference to this object for chaining calls
    auto setGeneratorRandomSizeEnabled(GeneratorRandomSizeEnabledType value) -> Parameters&;

    /// Sets the BufferParameters parameter
    ///
    /// Registers a memory (with BufferParameters::Memory) or file (with BufferParameters::File) buffer with the
    /// DMA channel.
    ///
    /// Note that if the IOMMU is not enabled, the buffer may not be presented as a contiguous physical space to the
    /// readout card. In this case, the user is responsible for ensuring that superpages given to the driver are
    /// physically contiguous.
    ///
    /// It is recommended to use hugepages for the buffer to increase contiguousness, for example by opening a
    /// MemoryMappedFile in a hugetlbfs filesystem.
    /// See the README.md file for more information about hugepages.
    ///
    /// There is also a BufferParameters::Null option, which can be used to instantiate the DmaChannel without
    /// initiating data transfer, e.g. for testing purposes.
    ///
    /// Required parameter for the C-RORC and CRU drivers.
    ///
    /// \param value The value to set
    /// \return Reference to this object for chaining calls
    auto setBufferParameters(BufferParametersType value) -> Parameters&;

    /// Sets the ReadoutMode parameter
    ///
    /// This is a work-in-progress feature. Currently, only the C-RORC is supported in continuous readout mode.
    ///
    /// \param value The value to set
    /// \return Reference to this object for chaining calls
    auto setReadoutMode(ReadoutModeType value) -> Parameters&;

    /// Sets the LinkMask parameter
    ///
    /// The BAR channel may transfer data from multiple links.
    /// When this parameter is set, the links corresponding to the given number are enabled.
    /// When this parameter is not set, ??? links will be enabled (none? one? to be determined...)
    ///
    /// When an invalid link is given, the DMA channel may throw an InvalidLinkId exception.
    ///
    /// Note: the linkMaskFromString() function may be used to convert a std::string to a LinkMaskType that can be
    /// passed to this setter, which may be convenient when converting from string-based configuration values.
    ///
    /// \param value The value to set
    /// \return Reference to this object for chaining calls
    auto setLinkMask(LinkMaskType value) -> Parameters&;


    // Non-throwing getters

    /// Gets the CardId parameter
    /// \return The value wrapped in an optional if it is present, or an empty optional if it was not
    auto getCardId() const -> boost::optional<CardIdType>;

    /// Gets the ChannelNumber parameter
    /// \return The value wrapped in an optional if it is present, or an empty optional if it was not
    auto getChannelNumber() const -> boost::optional<ChannelNumberType>;

    /// Gets the DmaPageSize parameter
    /// \return The value wrapped in an optional if it is present, or an empty optional if it was not
    auto getDmaPageSize() const -> boost::optional<DmaPageSizeType>;

    /// Gets the GeneratorEnabled parameter
    /// \return The value wrapped in an optional if it is present, or an empty optional if it was not
    auto getGeneratorEnabled() const -> boost::optional<GeneratorEnabledType>;

    /// Gets the GeneratorDataSize parameter
    /// \return The value wrapped in an optional if it is present, or an empty optional if it was not
    auto getGeneratorDataSize() const -> boost::optional<GeneratorDataSizeType>;

    /// Gets the GeneratorLoopback parameter
    /// \return The value wrapped in an optional if it is present, or an empty optional if it was not
    auto getGeneratorLoopback() const -> boost::optional<GeneratorLoopbackType>;

    /// Gets the GeneratorPattern parameter
    /// \return The value wrapped in an optional if it is present, or an empty optional if it was not
    auto getGeneratorPattern() const -> boost::optional<GeneratorPatternType>;

    /// Gets the GeneratorRandomSizeEnabled parameter
    /// \return The value wrapped in an optional if it is present, or an empty optional if it was not
    auto getGeneratorRandomSizeEnabled() const -> boost::optional<GeneratorRandomSizeEnabledType>;

    /// Gets the BufferParameters parameter
    /// \return The value wrapped in an optional if it is present, or an empty optional if it was not
    auto getBufferParameters() const -> boost::optional<BufferParametersType>;

    /// Gets the ReadoutMode parameter
    /// \return The value wrapped in an optional if it is present, or an empty optional if it was not
    auto getReadoutMode() const -> boost::optional<ReadoutModeType>;

    /// Gets the LinkMask parameter
    /// \return The value wrapped in an optional if it is present, or an empty optional if it was not
    auto getLinkMask() const -> boost::optional<LinkMaskType>;


    // Throwing getters

    /// Gets the CardId parameter
    /// \exception ParameterException The parameter was not present
    /// \return The value
    auto getCardIdRequired() const -> CardIdType;

    /// Gets the ChannelNumber parameter
    /// \exception ParameterException The parameter was not present
    /// \return The value
    auto getChannelNumberRequired() const -> ChannelNumberType;

    /// Gets the DmaPageSize parameter
    /// \exception ParameterException The parameter was not present
    /// \return The value
    auto getDmaPageSizeRequired() const -> DmaPageSizeType;

    /// Gets the GeneratorEnabled parameter
    /// \exception ParameterException The parameter was not present
    /// \return The value
    auto getGeneratorEnabledRequired() const -> GeneratorEnabledType;

    /// Gets the GeneratorDataSize parameter
    /// \exception ParameterException The parameter was not present
    /// \return The value
    auto getGeneratorDataSizeRequired() const -> GeneratorDataSizeType;

    /// Gets the GeneratorLoopback parameter
    /// \exception ParameterException The parameter was not present
    /// \return The value
    auto getGeneratorLoopbackRequired() const -> GeneratorLoopbackType;

    /// Gets the GeneratorPattern parameter
    /// \exception ParameterException The parameter was not present
    /// \return The value
    auto getGeneratorPatternRequired() const -> GeneratorPatternType;

    /// Gets the GeneratorRandomSizeEnabled parameter
    /// \exception ParameterException The parameter was not present
    /// \return The value
    auto getGeneratorRandomSizeEnabledRequired() const -> GeneratorRandomSizeEnabledType;

    /// Gets the BufferParameters parameter
    /// \exception ParameterException The parameter was not present
    /// \return The value
    auto getBufferParametersRequired() const -> BufferParametersType;

    /// Gets the ReadoutMode parameter
    /// \exception ParameterException The parameter was not present
    /// \return The value
    auto getReadoutModeRequired() const -> ReadoutModeType;

    /// Gets the LinkMask parameter
    /// \exception ParameterException The parameter was not present
    /// \return The value
    auto getLinkMaskRequired() const -> LinkMaskType;


    // Helper functions

    /// Convenience function to make a Parameters object with card ID and channel number, since these are the most
    /// frequently used parameters
    static Parameters makeParameters(CardIdType cardId, ChannelNumberType channel)
    {
      return Parameters().setCardId(cardId).setChannelNumber(channel);
    }

    /// Convert a string to a set of link IDs for the setLinkMask() function.
    /// Can contain comma separated integers or ranges. For example:
    /// * "0,1,2,8-10" for links 0, 1, 2, 8, 9 and 10
    /// * "0-19,21-23" for links 0 to 23 except 20
    /// \throw ParseException on failure to parse
    static LinkMaskType linkMaskFromString(const std::string& string);

    /// Convert a string to a CardIdType for the setCardId() function.
    /// Can contain an integer or PCI address. For example:
    /// * "12345"
    /// * "42:0.0"
    /// \throw ParseException on failure to parse
    /// \throw ParameterException on PciAddress numbers out of range
    static CardIdType cardIdFromString(const std::string& string);

  private:
    std::unique_ptr<ParametersPimpl> mPimpl;
};

} // namespace roc
} // namespace AliceO2

#endif // ALICEO2_INCLUDE_READOUTCARD_PARAMETERS_H_
