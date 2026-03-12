/***************
TBD-16 — Macro/Preset System & PicoSeqRack

(c) 2025-2026 Per-Olov Jernberg (possan). https://possan.codes

Licensed under the GNU Lesser General Public License (LGPL 3.0).
https://www.gnu.org/licenses/lgpl-3.0.txt

dadamachines has a commercial license to use this code in the TBD-16 product.
Other commercial use requires a separate license agreement.

Provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#pragma once

#include "SpiProtocol.h"
#include <stdint.h>

class SpiProtocolHelper {
private:
  bool nextResponsePrepared;
  bool canPrepareNextResponse;

public:
  SpiProtocolHelper();
  bool shouldPrepareNextResponse();
  void markNextResponsePrepared(p4_spi_response_header *header, p4_spi_response2 *response);

  void updateResponseBeforeSending(p4_spi_response_header *header, p4_spi_response2 *response);
  bool shouldSendPreparedResponse();
  void queuedPreparedResponse();

  bool validateRequestPacket(p4_spi_request_header *header, p4_spi_request2 *request);

  uint16_t calcPayloadCrc(uint8_t *data, uint16_t length);
  uint8_t nextResponseSequenceCounter;
  uint8_t lastSeenRequestCounter;

  uint8_t getNextSequence(uint8_t currentNumber);
  void markRequestSeen(uint8_t seq);
};

