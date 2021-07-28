/*!
 * FrameReceiverCameraLinkRxThread.cpp
 *
 *  Created on: July 27, 2021
 *      Author: Tim Nicholls, STFC Detector Systems Software Group
 */

#include <unistd.h>

#include "FrameReceiverCameraLinkRxThread.h"

using namespace FrameReceiver;

FrameReceiverCameraLinkRxThread::FrameReceiverCameraLinkRxThread(FrameReceiverConfig& config,
                                                   SharedBufferManagerPtr buffer_manager,
                                                   FrameDecoderPtr frame_decoder,
                                                   unsigned int tick_period_ms) :
    FrameReceiverRxThread(config, buffer_manager, frame_decoder, tick_period_ms),
    logger_(log4cxx::Logger::getLogger("FR.CameraLinkRxThread"))
{
  LOG4CXX_DEBUG_LEVEL(1, logger_, "FrameReceiverCameraLinkRxThread constructor entered....");

  // Store the frame decoder as a CameraLink type frame decoder
  frame_decoder_ = boost::dynamic_pointer_cast<FrameDecoderCameraLink>(frame_decoder);
}

FrameReceiverCameraLinkRxThread::~FrameReceiverCameraLinkRxThread()
{
  LOG4CXX_DEBUG_LEVEL(1, logger_, "Destroying FrameReceiverCameraLinkRxThread....");
}

void FrameReceiverCameraLinkRxThread::run_specific_service(void)
{
  LOG4CXX_DEBUG_LEVEL(1, logger_, "Running CameraLink RX thread service");
}

void FrameReceiverCameraLinkRxThread::cleanup_specific_service(void)
{
  LOG4CXX_DEBUG_LEVEL(1, logger_, "Cleaning up CameraLink RX thread service");
}
