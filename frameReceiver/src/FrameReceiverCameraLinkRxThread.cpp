/*!
 * FrameReceiverCameraLinkRxThread.cpp
 *
 * FrameReceiver RX thread implementation for CameraLink-based systems. Extends the low-level RX
 * thead to faciliate acquisition of images by a separate canera service thread provided by
 * frame decoders. An additional 'notifier' IPC channel is created to receive buffer ready
 * notifications from the camera service thread.
 *
 *  Created on: July 27, 2021
 *      Author: Tim Nicholls, STFC Detector Systems Software Group
 */

#include <unistd.h>

#include "FrameReceiverCameraLinkRxThread.h"

using namespace FrameReceiver;

//! Constructor for the FrameReceiverCameraLinkRxThread class.
//!
//! This method initialises the CameraLink RX thread class, creating the additional notifier
//! IPC channel.
//!
//! \param[in] config - FrameReceiver configuration object
//! \param[in] buffer_manager - pointer to the current shared buffer manager instance
//! \param[in] frame_decoder - pointer to the current frame decoder instance
//! \param[in] tick_period_ms - unerlying RX thread reactor tick interval in milliseconds
//!
FrameReceiverCameraLinkRxThread::FrameReceiverCameraLinkRxThread(FrameReceiverConfig& config,
                                                   SharedBufferManagerPtr buffer_manager,
                                                   FrameDecoderPtr frame_decoder,
                                                   unsigned int tick_period_ms) :
    FrameReceiverRxThread(config, buffer_manager, frame_decoder, tick_period_ms),
    logger_(log4cxx::Logger::getLogger("FR.CameraLinkRxThread")),
    notify_channel_(ZMQ_PAIR),
    notify_endpoint_(Defaults::default_notify_endpoint)
{
  LOG4CXX_DEBUG_LEVEL(1, logger_, "FrameReceiverCameraLinkRxThread constructor entered....");

  // Store the frame decoder as a CameraLink type frame decoder
  frame_decoder_ = boost::dynamic_pointer_cast<FrameDecoderCameraLink>(frame_decoder);
}

//! Destructor for the FrameReceiverCameraLinkRxThread class
//!
FrameReceiverCameraLinkRxThread::~FrameReceiverCameraLinkRxThread()
{
  LOG4CXX_DEBUG_LEVEL(1, logger_, "Destroying FrameReceiverCameraLinkRxThread....");
}

//! Run the specific service implementation for the CamaeraLink RX thread class
//!
//! This method starts the class-specific service run in the RX thread class. The notifier channel
//! is bound to its endpoint and added to the RX thread reactor and then the camera-specifc
//! acquisition service is launched in the frame decoder.
//!
void FrameReceiverCameraLinkRxThread::run_specific_service(void)
{
  LOG4CXX_DEBUG_LEVEL(1, logger_, "Running CameraLink RX thread service");

  // Bind the notify channel to its endpoint
  try
  {
    LOG4CXX_DEBUG_LEVEL(1, logger_, "Binding notify channel to endpoint " << notify_endpoint_);
    notify_channel_.bind(notify_endpoint_);
  }
  catch (zmq::error_t& e)
  {
    std::stringstream ss;
    ss << "Binding notify channel to endpoint " << notify_endpoint_ << "failed: " << e.what();
      this->set_thread_init_error(ss.str());
      return;
  }

  // Add the notify channel to the reactor
  reactor_.register_channel(notify_channel_,
    boost::bind(&FrameReceiverCameraLinkRxThread::handle_notify_channel, this));

  // Start the camera-specific service implemented by the decoder
  frame_decoder_->start_camera_service(notify_endpoint_, reactor_);
}

//! Handle events on the RX thread notifier channel
//!
//! This method is called by the RX thread reactor to handle events on the notifier channel.
//! Incoming frame ready notifications from the camera service thread and forwarded to the main
//! application thread via the RX channel. All other messages are logged as errors and discarded.
//!
void FrameReceiverCameraLinkRxThread::handle_notify_channel(void)
{
  std::string notify_msg_encoded = notify_channel_.recv();

  try
  {
    IpcMessage notify_msg(notify_msg_encoded.c_str());
    IpcMessage::MsgType msg_type = notify_msg.get_msg_type();
    IpcMessage::MsgVal msg_val = notify_msg.get_msg_val();

    if ((msg_type == IpcMessage::MsgTypeNotify) && (msg_val == IpcMessage::MsgValNotifyFrameReady))
    {
      LOG4CXX_DEBUG_LEVEL(2, logger_, "Got frame ready notification from notify channel"
        << " for frame " << notify_msg.get_param<int>("frame", -1)
        << " in buffer  " << notify_msg.get_param<int>("buffer_id", -1)
      );
      rx_channel_.send(notify_msg_encoded);
    }
    else
    {
      LOG4CXX_ERROR(logger_, "Got unexpected messsage on notify channel: " << notify_msg_encoded);
    }
  }
  catch (IpcMessageException& e)
  {
    LOG4CXX_ERROR(logger_, "Error decoding notify channel message: " << e.what());
  }
}

//! Clean up the RX thread service
//!
//! This method is called to clean up the RX thread service on shutdown, stopping frame decoder
//! camera service in turn.
//!
void FrameReceiverCameraLinkRxThread::cleanup_specific_service(void)
{
  LOG4CXX_DEBUG_LEVEL(1, logger_, "Cleaning up CameraLink RX thread service");
  frame_decoder_->stop_camera_service();
}
