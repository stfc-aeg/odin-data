/*
 * FrameDecoderCameraLink.cpp
 *
 * Abstract base class for frameReceiver decoder implementations for CameraLink-based
 * systems.
 *
 *  Created on: July 27, 2021
 *      Author: Tim Nicholls, STFC Detector Sytems Software Group
 */
#include "FrameDecoderCameraLink.h"

const std::string default_camera_ctrl_endpoint = "tcp://*:5060";

using namespace FrameReceiver;
using namespace OdinData;

//! Constructor for the FrameDecoderCameraLink class.
//!
//! This method initialises the base class of each CameraLink base class, setting
//! default values for base confoiguration parameters and variables.
//!
FrameDecoderCameraLink::FrameDecoderCameraLink() :
  FrameDecoder(),
  ctrl_channel_(ZMQ_ROUTER),
  ctrl_endpoint_(default_camera_ctrl_endpoint),
  run_thread_(false),
  notify_channel_(ZMQ_PAIR)
{
};

//! Initialise the FrameDecoderCameraLink instance
//!
//! This method initialises the base FrameDecoderCameraLink instance, extracting and storing
//! parameters from the configuration message.
//!
//! \param[in] config_msg - IpcMessage containing decoder configuration parameters
//!
void FrameDecoderCameraLink::init(OdinData::IpcMessage& config_msg)
{
  // Pass the configuration message to the base class initialisation method
  FrameDecoder::init(config_msg);

  // Update configuration parameters if found in the config message
  ctrl_endpoint_ = config_msg.get_param<std::string>(
    CONFIG_DECODER_CAMERA_CTRL_ENDPOINT, ctrl_endpoint_
  );

}

//! Start the decoder camera service
//!
//! This method starts the decoder camera service. Called from the context of the receiver thread
//! this binds the camera control channel and registers it with the receiver thread reactor and
//! then launches a new thread running the camera service.
//!
//! \param[in] notify_endpoint - IpcChannel endpoint for cross-thread buffer ready notification
//! \param[in] rxthread_reactor - RX thread reactor to register camera control thread with
//!
void FrameDecoderCameraLink::start_camera_service(
  std::string& notify_endpoint, OdinData::IpcReactor& rxthread_reactor)
{

  LOG4CXX_DEBUG_LEVEL(1, logger_, "Starting camera service");

  notify_endpoint_ = notify_endpoint;

  // Bind the camera control channel to the endpoint
  try
  {
    LOG4CXX_DEBUG_LEVEL(2, logger_, "Binding camera control channel to endpoint: " << ctrl_endpoint_);
    ctrl_channel_.bind(ctrl_endpoint_);
  }
  catch (zmq::error_t e)
  {
    std::stringstream sstr;
    sstr << "Binding control channel endpoint " << ctrl_endpoint_ << " failed: " << e.what();
    throw OdinData::OdinDataException(sstr.str());
  }

  // Register the control channel with the RX thread reactor
  rxthread_reactor.register_channel(ctrl_channel_,
    boost::bind(&FrameDecoderCameraLink::handle_ctrl_channel, this)
  );

  // Start the camera service thread
  run_thread_ = true;
  camera_thread_.reset(new boost::thread(
    boost::bind(&FrameDecoderCameraLink::run_camera_service_internal, this)
  ));

}

//! Stop the camera service thread
//!
//! this method stops the running camera service thread. The run_thread_ flag is set to flase,
//! allowing the thread event loop to terminate cleanly and then the thread is joined.
//!
void FrameDecoderCameraLink::stop_camera_service(void)
{
  LOG4CXX_DEBUG_LEVEL(1, logger_, "Stopping camera service thread");

  run_thread_ = false;
  if (camera_thread_) {
    LOG4CXX_DEBUG_LEVEL(2, logger_, "Waiting for camera service thread to stop....");
    camera_thread_->join();
  }

  LOG4CXX_DEBUG_LEVEL(1, logger_, "Camera service thread stopped");
}


//! Push a buffer onto the empty buffer queue
//!
//! This method adds an empty shared memory buffer onto the decoder internal empty buffer
//! queue. This is simply a call to the base class method wrapped in a mutex to allow the
//! queue to be safely used across the context of both the RX and camera controller threads.
//!
//! \param[in] buffer_id - SharedBufferManager buffer ID
//!
void FrameDecoderCameraLink::push_empty_buffer(int buffer_id)
{
  boost::lock_guard<boost::mutex> lock(mutex_);
  FrameDecoder::push_empty_buffer(buffer_id);
}

//! Get a buffer from the empty buffer queue
//!
//! This method checks if a buffer ie available on the decoder internal empty buffer queue
//! and, if so, pops one off the queue and determines the buffer id and address. This is
//! wrapped in a mutex to allow thread-safe use.
//!
//! TODO the underlying logic should be devolved to the base FrameDecoder class
//!
//! \param[out] buffer_id - ID of next empty buffer from queue
//! \param[out] buffer_addr - address of next emptty buffer from queue
//! \return boolean value, true if buffer available, false if not
//!
bool FrameDecoderCameraLink::get_empty_buffer(int& buffer_id, void*& buffer_addr)
{
  boost::lock_guard<boost::mutex> lock(mutex_);

  // Check if queue is not empty
  bool buffer_avail = !empty_buffer_queue_.empty();

  // If buffer available, get ID and address and pop off the front of the queue
  if (buffer_avail)
  {
    buffer_id = empty_buffer_queue_.front();
    empty_buffer_queue_.pop();
    buffer_addr = buffer_manager_->get_buffer_address(buffer_id);
  }

  return buffer_avail;
}

//! Notify the RX thread that a filled buffer is ready
//!
//! This protected method is used to allow the camera service thread in a decoder
//! to notify the RX thread that a filled buffer is ready to be passed on for
//! downstream processing.
//!
//! \param[in] buffer_id - ID of the shared buffer
//! \param[in] frame_namber - frame number filled into the shared buffer
//!
void FrameDecoderCameraLink::notify_frame_ready(int buffer_id, int frame_number)
{
  LOG4CXX_DEBUG_LEVEL(2, logger_,
    "Notifying frame " << frame_number << " ready in buffer " << buffer_id);

  IpcMessage ready_msg(IpcMessage::MsgTypeNotify, IpcMessage::MsgValNotifyFrameReady);
  ready_msg.set_param("frame", frame_number);
  ready_msg.set_param("buffer_id", buffer_id);

  notify_channel_.send(ready_msg.encode());

}

//! Internal camera service thread runner
//!
//! This private method implements the camera service thread. This first connects the buffer
//! notifier channel to the RX thread and then calls the decoder-specific camera service method.
//!
void FrameDecoderCameraLink::run_camera_service_internal(void)
{

// Connect the notify channel to the RX thread
  try
  {
    LOG4CXX_DEBUG_LEVEL(1, logger_,
      "Connecting notify channel to endpoint " << notify_endpoint_);
    notify_channel_.connect(notify_endpoint_);
  }
  catch (zmq::error_t& e)
  {
    std::stringstream ss;
    ss << "Connecting notify channel to endpoint " << notify_endpoint_
      << " failed: " << e.what();
    //this->set_thread_init_error(ss.str());
    return;
  }

  // Run the decoder-specific camera service
  this->run_camera_service();

}

