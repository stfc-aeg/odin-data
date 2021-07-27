/*
 * FrameDecoderCameraLink.h
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

  virtual ~FrameDecoderCameraLink() = 0;

};

inline FrameDecoderCameraLink::~FrameDecoderCameraLink() {};

typedef boost::shared_ptr<FrameDecoderCameraLink> FrameDecoderCameraLinkPtr;

} // namespace FrameReceiver
#endif /* INCLUDE_FRAMEDECODER_CAMERALINK_H_ */
