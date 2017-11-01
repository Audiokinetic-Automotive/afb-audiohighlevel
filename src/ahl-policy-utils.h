/*
 * Copyright (C) 2017 "Audiokinetic Inc"
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AHL_POLICY_UTILS_INCLUDE
#define AHL_POLICY_UTILS_INCLUDE

#define AFB_BINDING_VERSION 2
#include <json-c/json.h>
#include <afb/afb-binding.h>
#include <glib.h>

#define AHL_POLICY_ACCEPT 1
#define AHL_POLICY_REJECT 0
#define AHL_POLICY_UTIL_SUCCESS 0
#define AHL_POLICY_UTIL_FAIL 1

#define AHL_UNDEFINED -1

typedef int endpointID_t;
typedef int streamID_t;

typedef enum StreamEvent {
    STREAM_EVENT_START  = 0,   // Stream is inactive
    STREAM_EVENT_STOP,         // Stream is running    
    STREAM_EVENT_PAUSE,        // Audio stream paused
    STREAM_EVENT_RESUME,       // Audio stream resumed
    STREAM_EVENT_MUTED,        // Audio stream muted
    STREAM_EVENT_UNMUTED,      // Audio stream unmuted
    STREAM_EVENT_MAXVALUE      // Enum count, keep at the end
} StreamEventT;

typedef enum StreamMute {
    STREAM_UNMUTED = 0,         // Stream is not muted
    STREAM_MUTED,               // Stream is muted
    STREAM_MUTE_MAXVALUE,       // Enum count, keep at the end
} StreamMuteT;

typedef enum EndpointType {
    ENDPOINTTYPE_SOURCE = 0,        // source devices
    ENDPOINTTYPE_SINK,              // sink devices
    ENDPOINTTYPE_MAXVALUE           // Enum count, keep at the end
} EndpointTypeT;

typedef enum StreamState {
    STREAM_STATE_IDLE  = 0,    // Stream is inactive
    STREAM_STATE_RUNNING,      // Stream is active and running    
    STREAM_STATE_PAUSED,       // Stream is active but paused
    STREAM_STATE_MAXVALUE      // Enum count, keep at the end
} StreamStateT;

// Define default behavior of audio role when interrupting lower priority sources
typedef enum InterruptBehavior {
  INTERRUPTBEHAVIOR_CONTINUE = 0, // Continue to play lower priority source when interrupted (e.g. media may be ducked)
  INTERRUPTBEHAVIOR_CANCEL,       // Abort playback of lower priority source when interrupted (e.g. non-important HMI feedback that does not make sense later)
  INTERRUPTBEHAVIOR_PAUSE,        // Pause lower priority source when interrupted, to be resumed afterwards (e.g. non-temporal guidance)
  INTERRUPTBEHAVIOR_MAXVALUE,     // Enum count, keep at the end
} InterruptBehaviorT;

typedef enum DeviceURIType {
    DEVICEURITYPE_ALSA_HW = 0,  // Alsa hardware device URI
    DEVICEURITYPE_ALSA_DMIX,    // Alsa Dmix device URI (only for playback devices)
    DEVICEURITYPE_ALSA_DSNOOP,  // Alsa DSnoop device URI (only for capture devices)
    DEVICEURITYPE_ALSA_SOFTVOL, // Alsa softvol device URI
    DEVICEURITYPE_ALSA_PLUG,    // Alsa plug device URI
    DEVICEURITYPE_ALSA_OTHER,   // Alsa domain URI device of unspecified type
    DEVICEURITYPE_NOT_ALSA,     // Unknown (not ALSA domain)
    DEVICEURITYPE_MAXVALUE      // Enum count, keep at the end
} DeviceURITypeT;

// CPU endianness assumed in all formats
typedef enum SampleType {
  AHL_FORMAT_UNKNOWN = -1,              // Unknown
  AHL_FORMAT_U8 = 0,                    // Unsigned 8 bit
  AHL_FORMAT_S16,                       // Signed 16 bit Little Endian
  AHL_FORMAT_S24,                       // Signed 24 bit Little Endian using low three bytes in 32-bit word
  AHL_FORMAT_S32,                       // Signed 32 bit Little Endian
  AHL_FORMAT_FLOAT,                     // Float 32 bit Little Endian, Range -1.0 to 1.0
  AHL_FORMAT_FLOAT64,                   // Float 64 bit Little Endian, Range -1.0 to 1.0
  AHL_FORMAT_IEC958,                    // IEC-958 Little Endian (SPDIF)
  AHL_FORMAT_MU_LAW,                    // Mu-Law
  AHL_FORMAT_A_LAW,                     // A-Law
  AHL_FORMAT_IMA_ADPCM,                 // Ima-ADPCM
  AHL_FORMAT_MPEG,                      // MPEG
  AHL_FORMAT_GSM,                       // GSM
  AHL_FORMAT_G723,                      // G723
  AHL_FORMAT_DSD,                       // Direct stream digital
  AHL_FORMAT_MAXVALUE,                  // Enum count, keep at the end
} SampleTypeT;

typedef struct AudioFormat {
    int         sampleRate;     // Sample rate
    int         numChannels;    // Number of channels
    SampleTypeT sampleType;     // Sample type
    // TODO: Interleaving?
    // TODO: Sample sub format?
} AudioFormatT;

typedef struct AlsaDeviceInfo {
    int             cardNum;            // HW card number
    int             deviceNum;          // HW device number                                 
    int             subDeviceNum;       // HW sub device number
} AlsaDeviceInfoT;

typedef enum EndpointSelectionMode {
    ENDPOINTSELMODE_AUTO = 0,          // Automatic endpoint selection based on config priority
    ENDPOINTSELMODE_MANUAL,             // Explicit endpoint selection
    ENDPOINTSELMODEMAXVALUE,            // Enum count, keep at the end
} EndpointSelectionModeT;


typedef struct EndPointInterfaceInfo {
    endpointID_t    endpointID;         // Unique endpoint ID (per type)
    EndpointTypeT   type;               // Source or sink device
    char *          gsDeviceName;       // Unique device card name 
    char *          gsDisplayName;      // Application display name
    char *          gsDeviceURI;        // Associated URI 
    char *          gsDeviceDomain;     // Device URI domain (e.g. alsa or pulse)
    DeviceURITypeT  deviceURIType;      // Device URI type (includes audio domain information)
    char *          gsHALAPIName;       // HAL associated with the device (for volume control)
    char *          pRoleName;          // Role string identifier (from role config but could be programatically overriden later)
    AlsaDeviceInfoT alsaInfo;           // ALSA specific device information
    AudioFormatT    format;             // Preferred audio format supported (later could be array of supported formats)
    int             iVolume;            // Storage for current endpoint volume (policy effected).  
    json_object     *pPropTableJ;       //Property Table           
} EndPointInterfaceInfoT;

 
typedef struct StreamInterfaceInfo {
    streamID_t              streamID;           // Stream unique ID
    StreamStateT            streamState;        // Stream activity state
    StreamMuteT             streamMute;         // Stream mute state
    char *                  pRoleName;          // Role string identifier (from role config but could be programatically overriden later)
    int                     iPriority;          // Role normalized priority (0-100) (from role config but could be programatically overriden later)
    InterruptBehaviorT      eInterruptBehavior; // Role behavior when interrupting lower priority streams (from role config but could be programatically overriden later)    
    EndPointInterfaceInfoT  endpoint;
} StreamInterfaceInfoT;

void Add_Endpoint_Property_Double( json_object * io_pPropertyArray, char * in_pPropertyName, double in_dPropertyValue);
void Add_Endpoint_Property_Int( json_object * io_pPropertyArray, char * in_pPropertyName, int in_iPropertyValue);
void Add_Endpoint_Property_String( json_object * io_pPropertyArray, char * in_pPropertyName, const char * in_pPropertyValue);
int EndpointToJSON(EndPointInterfaceInfoT * pEndpoint, json_object **ppEndpointJ);
int JSONToEndpoint(json_object *pEndpointJ, EndPointInterfaceInfoT * pStream);
int StreamToJSON(StreamInterfaceInfoT * pPolicyStream, json_object **ppStreamJ);
int JSONToStream(json_object *pStreamJ, StreamInterfaceInfoT * pPolicyStream);

#endif // AHL_POLICY_UTILS_INCLUDE
