/*!
 * FrameReceiverCameraLinkThread.h
 *
 *  Created on: July 27, 2021
 *      Author: Tim Nicholls, STFC Detector Systems software Group
 */

#ifndef FRAMERECEIVERCAMERALINKRXTHREAD_H_
#define FRAMERECEIVERCAMERALINKRXTHREAD_H_

#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include <log4cxx/logger.h>
using namespace log4cxx;
using namespace log4cxx::helpers;
#include "DebugLevelLogger.h"

#include "IpcChannel.h"
#include "IpcMessage.h"
#include "IpcReactor.h"
#include "SharedBufferManager.h"
#include "FrameDecoderCameraLink.h"
#include "FrameReceiverConfig.h"
#include "FrameReceiverRxThread.h"
#include "OdinDataException.h"

using namespace OdinData;

namespace FrameReceiver
{
class FrameReceiverCameraLinkRxThread : public FrameReceiverRxThread
{
public:
  FrameReceiverCameraLinkRxThread(FrameReceiverConfig& config, SharedBufferManagerPtr buffer_manager,
      FrameDecoderPtr frame_decoder, unsigned int tick_period_ms=100);
  virtual ~FrameReceiverCameraLinkRxThread();

private:

  void run_specific_service(void);
  void cleanup_specific_service(void);

  LoggerPtr                 logger_;
  FrameDecoderCameraLinkPtr frame_decoder_;

};

} // namespace FrameReceiver
#endif /* FRAMERECEIVERCAMERALINKRXTHREAD_H_ */
