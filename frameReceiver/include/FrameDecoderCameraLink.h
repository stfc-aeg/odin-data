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
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>

#include <log4cxx/logger.h>
using namespace log4cxx;
using namespace log4cxx::helpers;
#include "DebugLevelLogger.h"

#include "FrameDecoder.h"

namespace FrameReceiver
{

class FrameDecoderCameraLink : public FrameDecoder
{
public:

  FrameDecoderCameraLink() :
      FrameDecoder()
  {
  };

  virtual void start_camera_service(std::string& notify_endpoint) = 0;
  virtual void stop_camera_service(void) = 0;
  virtual ~FrameDecoderCameraLink() = 0;

  void push_empty_buffer(int buffer_id)
  {
    boost::lock_guard<boost::mutex> lock(mutex_);
    FrameDecoder::push_empty_buffer(buffer_id);
  }

  bool get_empty_buffer(int& buffer_id, void*& buffer_addr)
  {
    boost::lock_guard<boost::mutex> lock(mutex_);

    bool buffer_avail = !empty_buffer_queue_.empty();

    if (buffer_avail)
    {
      buffer_id = empty_buffer_queue_.front();
      empty_buffer_queue_.pop();
      buffer_addr = buffer_manager_->get_buffer_address(buffer_id);
    }

    return buffer_avail;
  }

private:
  boost::mutex mutex_;

};

inline FrameDecoderCameraLink::~FrameDecoderCameraLink() {};

typedef boost::shared_ptr<FrameDecoderCameraLink> FrameDecoderCameraLinkPtr;

} // namespace FrameReceiver
#endif /* INCLUDE_FRAMEDECODER_CAMERALINK_H_ */
