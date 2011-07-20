// Copyright (c) 2011 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
#ifndef HTTP_CLIENT_WEBM_ENCODER_DSHOW_H_
#define HTTP_CLIENT_WEBM_ENCODER_DSHOW_H_

#pragma once

#include <map>
#include <string>

#include <comdef.h>
// files included by dshow.h cause many 4996 warnings
#pragma warning(disable:4996)
#include <dshow.h>
#pragma warning(default:4996)

#include "boost/shared_ptr.hpp"
#include "boost/thread/thread.hpp"
#include "chromium/base/basictypes.h"
#include "http_client_base.h"
#include "webm_encoder.h"

namespace WebmLive {
// A slightly more brief version of the com_ptr_t definition macro.
#define COMPTR_TYPEDEF(InterfaceName) \
  _COM_SMARTPTR_TYPEDEF(InterfaceName, IID_##InterfaceName)
COMPTR_TYPEDEF(IBaseFilter);
COMPTR_TYPEDEF(ICaptureGraphBuilder2);
COMPTR_TYPEDEF(ICreateDevEnum);
COMPTR_TYPEDEF(IEnumMediaTypes);
COMPTR_TYPEDEF(IEnumMoniker);
COMPTR_TYPEDEF(IEnumPins);
COMPTR_TYPEDEF(IFilterGraph);
COMPTR_TYPEDEF(IFileSinkFilter2);
COMPTR_TYPEDEF(IGraphBuilder);
COMPTR_TYPEDEF(IMediaControl);
COMPTR_TYPEDEF(IMediaEvent);
COMPTR_TYPEDEF(IMediaSeeking);
COMPTR_TYPEDEF(IMoniker);
COMPTR_TYPEDEF(IPin);
COMPTR_TYPEDEF(IPropertyBag);

// CLSID constants for directshow filters needed to encode WebM files.
// Xiph.org Vorbis encoder CLSID
const CLSID CLSID_VorbisEncoder = {
  // 5C94FE86-B93B-467F-BFC3-BD6C91416F9B
  0x5C94FE86,
  0xB93B,
  0x467F,
  {0xBF, 0xC3, 0xBD, 0x6C, 0x91, 0x41, 0x6F, 0x9B}
};
// Webmdshow project color conversion filter CLSID.  Not used at present.
const CLSID CLSID_WebmColorConversion = {
  // ED311140-5211-11DF-94AF-0026B977EEAA
  0xED311140,
  0x5211,
  0x11DF,
  {0x94, 0xAF, 0x00, 0x26, 0xB9, 0x77, 0xEE, 0xAA}
};
// Webmdshow project muxer filter CLSID.
const CLSID CLSID_WebmMux = {
  // ED3110F0-5211-11DF-94AF-0026B977EEAA
  0xED3110F0,
  0x5211,
  0x11DF,
  {0x94, 0xAF, 0x00, 0x26, 0xB9, 0x77, 0xEE, 0xAA}
};
// Webmdshow project VP8 encoder filter CLSID.
const CLSID CLSID_VP8Encoder = {
  // ED3110F5-5211-11DF-94AF-0026B977EEAA
  0xED3110F5,
  0x5211,
  0x11DF,
  {0x94, 0xAF, 0x00, 0x26, 0xB9, 0x77, 0xEE, 0xAA}
};

// Time conversion utility function for conversion of 100 nanosecond units to
// seconds.
double media_time_to_seconds(REFERENCE_TIME media_time);

// WebM encoder object. Currently supports only live encoding from the primary
// video and audio input devices on the user system.
class WebmEncoderImpl {
 public:
  enum {
    // Internal status codes for the DirectShow encoder.
    // Graph abort event received.
    kGraphAborted = -219,
    // Unable to configure the graph.
    kGraphConfigureError = -215,
    // Unable to connect the file writer filter.
    kFileWriterConnectError = -214,
    // Unable to create the file writer filter.
    kCannotCreateFileWriter = -213,
    // Unable to connect Vorbis encoder to WebM muxer.
    kWebmMuxerAudioConnectError = -212,
    // Unable to connect VP8 encoder to WebM muxer.
    kWebmMuxerVideoConnectError = -211,
    // Error configuring WebM muxer.
    kWebmMuxerConfigureError = -210,
    // Unable to obtain WebM muxer configuration interface.
    kCannotConfigureWebmMuxer = -209,
    // Unable to connect audio source to Vorbis encoder.
    kAudioConnectError = -208,
    // Unable to connect video source to VP8 encoder.
    kVideoConnectError = -207,
    // Error configuring VP8 encoder.
    kVpxConfigureError = -206,
    // Unable to obtain VP8 encoder configuration interface.
    kCannotConfigureVpxEncoder = -205,
    // Unable to add a filter to the graph.
    kCannotAddFilter = -204,
    // Unable to create the Vorbis encoder filter.
    kCannotCreateVorbisEncoder = -203,
    // Unable to create the VP8 encoder filter.
    kCannotCreateVpxEncoder = -202,
    // Unable to create the WebM muxer filter.
    kCannotCreateWebmMuxer = -201,
    // Unable to create graph interfaces.
    kCannotCreateGraph = -200,
    kSuccess = 0,
    // Graph completion event received.
    kGraphCompleted = 1,
  };
  WebmEncoderImpl();
  ~WebmEncoderImpl();
  // Creates WebM encoder graph. Returns |kSuccess| upon success, or a
  // |WebmEncoder| status code upon failure.
  int Init(const std::wstring& out_file_name);
  // Runs encoder thread. Returns |kSuccess| upon success, or a |WebmEncoder|
  // status code upon failure.
  int Run();
  // Stops encoder thread.
  void Stop();
  // Returns encoded duration in seconds.
  double encoded_duration();
 private:
  // Returns true when user wants the encode thread to stop.
  bool StopRequested();
  // Creates filter graph and graph builder interfaces.
  int CreateGraph();
  // Loads first available video capture source and adds it to the graph.
  int CreateVideoSource(std::wstring video_src);
  // Loads the VP8 encoder and adds it to the graph.
  int CreateVpxEncoder();
  // Connects video source to VP8 encoder.
  int ConnectVideoSourceToVpxEncoder();
  // Loads first available audio capture source and adds it to the graph.
  int CreateAudioSource(std::wstring video_src);
  // Loads the Vorbis encoder and adds it to the graph.
  int CreateVorbisEncoder();
  // Connects audio source to Vorbis encoder.
  int ConnectAudioSourceToVorbisEncoder();
  // Loads the WebM muxer and adds it to the graph.
  int CreateWebmMuxer();
  // Connects VP8 and vorbis encoders to the WebM muxer.
  int ConnectEncodersToWebmMuxer();
  // Creates the file writer filter and adds it to the graph.
  int CreateFileWriter();
  // Connects WebM muxer to file writer.
  int ConnectWebmMuxerToFileWriter();
  // Checks graph media event for error or completion.
  int HandleMediaEvent();
  // Updates encoded duration value.
  void set_encoded_duration(double current_duration);
  // WebM encoder thread function.
  void WebmEncoderThread();
  // Stop flag used by |Stop| and |StopRequested|.
  bool stop_;
  // Encoded duration.
  double encoded_duration_;
  // Handle to graph media event. Used to check for graph error and completion.
  HANDLE media_event_handle_;
  // Graph builder interfaces
  IGraphBuilderPtr graph_builder_;
  ICaptureGraphBuilder2Ptr capture_graph_builder_;
  // Directshow filters used in the encoder graph.
  IBaseFilterPtr audio_source_;
  IBaseFilterPtr video_source_;
  IBaseFilterPtr vorbis_encoder_;
  IBaseFilterPtr vpx_encoder_;
  IBaseFilterPtr webm_muxer_;
  IBaseFilterPtr file_writer_;
  // Graph control interface.
  IMediaControlPtr media_control_;
  // Media event interface used when |media_event_handle_| is signaled.
  IMediaEventPtr media_event_;
  // Graph seek interface used to obtain encoded duration.
  IMediaSeekingPtr media_seeking_;
  // Mutex providing synchronization between user interface and encoder thread.
  boost::mutex mutex_;
  // Encoder thread object.
  boost::shared_ptr<boost::thread> encode_thread_;
  // Output file name.
  std::wstring out_file_name_;
  DISALLOW_COPY_AND_ASSIGN(WebmEncoderImpl);
};

// Utility class for finding and loading capture devices available through
// DirectShow on user systems.
class CaptureSourceLoader {
 public:
  enum {
    kNoDeviceFound = -300,
    kSuccess = 0,
  };
  CaptureSourceLoader();
  ~CaptureSourceLoader();
  // Initialize the loader for audio or video devices.  Must specify either
  // CLSID_AudioInputDeviceCategory or CLSID_VideoInputDeviceCategory.
  int Init(CLSID source_type);
  // Returns number of sources found by Init.
  int GetNumSources() const { return sources_.size(); };
  // Return source name for specified index.
  std::wstring GetSourceName(int index) { return sources_[index]; };
  // Return filter for capture source at specified index.
  IBaseFilterPtr GetSource(int index) const;
 private:
  // Finds and stores all source devices of |source_type_| in |sources_|.
  int FindAllSources();
  // Utility for returning the string property specified by |prop_name| stored
  // in |prop_bag|.
  std::wstring GetStringProperty(IPropertyBagPtr& prop_bag,
                                 std::wstring prop_name);
  // Type of sources to find.
  CLSID source_type_;
  // System input device enumerator.
  IEnumMonikerPtr source_enum_;
  // Map of sources.
  std::map<int, std::wstring> sources_;
  DISALLOW_COPY_AND_ASSIGN(CaptureSourceLoader);
};

// Utility class for finding a specific pin on a DirectShow filter.
class PinFinder {
 public:
  PinFinder();
  ~PinFinder();
  // Initialize pin finder.
  int Init(const IBaseFilterPtr& filter);
  // TODO(tomfinegan): generalize these with a FindPin that takes a comparator.
  // All Find methods return an empty IPinPtr if unsuccessful.
  // Returns audio input pin at index.
  IPinPtr FindAudioInputPin(int index) const;
  // Returns audio output pin at index.
  IPinPtr FindAudioOutputPin(int index) const;
  // Returns video input pin at index.
  IPinPtr FindVideoInputPin(int index) const;
  // Returns video output pin at index.
  IPinPtr FindVideoOutputPin(int index) const;
  // Returns stream input pint at index.
  IPinPtr FindStreamInputPin(int index) const;
  // Returns stream output pin at index.
  IPinPtr FindStreamOutputPin(int index) const;
  // Returns input pin at index.
  IPinPtr FindInputPin(int index) const;
 private:
  // Filter pin enumerator interface.
  IEnumPinsPtr pin_enum_;
  DISALLOW_COPY_AND_ASSIGN(PinFinder);
};

// Utility class for obtaining information about a pin.
class PinInfo {
 public:
  // Copies supplied pin to |pin_|.
  explicit PinInfo(const IPinPtr& pin);
  ~PinInfo();
  // Checks for availability of specified major type.
  bool HasMajorType(GUID major_type) const;
  // Returns true for pins with media type audio.
  bool IsAudio() const;
  // Returns true for input pins.
  bool IsInput() const;
  // Returns true for output pins.
  bool IsOutput() const;
  // Returns true for pins with media type video.
  bool IsVideo() const;
  // Returns true for pins with media type stream.
  bool IsStream() const;
 private:
  // Disallow construction without IPinPtr.
  PinInfo();
  // Utility function used to free media type pointers.
  void FreeMediaType(AM_MEDIA_TYPE* ptr_media_type) const;
  // Copy of |ptr_pin| from |Init|
  const IPinPtr pin_;
  DISALLOW_COPY_AND_ASSIGN(PinInfo);
};

} // WebmLive

#endif  // HTTP_CLIENT_WEBM_ENCODER_DSHOW_H_