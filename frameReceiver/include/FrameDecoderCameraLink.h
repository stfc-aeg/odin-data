/*
 * FrameDecoderCameraLink.h
 *
 * Abstract base class for frameReceiver decoder implementations for CameraLink-based
 * systems.
 *
 *  Created on: July 27, 2021
 *      Author: Tim Nicholls, STFC Detector Sytems Software Group
 */

#ifndef INCLUDE_FRAMEDECODER_CAMERALINK_H_
#define INCLUDE_FRAMEDECODER_CAMERALINK_H_

#include <queue>
#include <map>

#include <stddef.h>
#include <stdint.h>
#include <netinet/in.h>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>

#include <log4cxx/logger.h>
using namespace log4cxx;
using namespace log4cxx::helpers;
#include "DebugLevelLogger.h"

#include "IpcChannel.h"
#include "IpcMessage.h"
#include "IpcReactor.h"

#include "FrameDecoder.h"

namespace FrameReceiver
{

  const std::string CONFIG_DECODER_CAMERA_CTRL_ENDPOINT = "camera_ctrl_endpoint";

  class FrameDecoderCameraLink : public FrameDecoder
  {
  public:

    FrameDecoderCameraLink();

    virtual ~FrameDecoderCameraLink() = 0;

    virtual void init(OdinData::IpcMessage& config_msg);
    virtual void start_camera_service(
      std::string& notify_endpoint, OdinData::IpcReactor& rxthread_reactor);
    virtual void stop_camera_service(void);
    virtual void handle_ctrl_channel(void) = 0;
    void push_empty_buffer(int buffer_id);
    bool get_empty_buffer(int& buffer_id, void*& buffer_addr);
    void notify_frame_ready(int buffer_id, int frame_number);

  protected:

    virtual void run_camera_service(void) = 0;

    OdinData::IpcChannel ctrl_channel_;
    std::string ctrl_endpoint_;
    bool run_thread_;

  private:

    void run_camera_service_internal(void);

    OdinData::IpcChannel notify_channel_;
    std::string notify_endpoint_;

    boost::mutex mutex_;
    boost::scoped_ptr<boost::thread> camera_thread_;

  };

  inline FrameDecoderCameraLink::~FrameDecoderCameraLink() {};

  typedef boost::shared_ptr<FrameDecoderCameraLink> FrameDecoderCameraLinkPtr;

} // namespace FrameReceiver
#endif /* INCLUDE_FRAMEDECODER_CAMERALINK_H_ */
