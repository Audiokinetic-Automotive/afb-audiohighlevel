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

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#define AFB_BINDING_VERSION 2
#include <afb/afb-binding.h>
#include <glib.h>
#include "wrap-json.h"
#include "ahl-policy-utils.h"
#include "ahl-interface.h"
#include "ahl-policy.h"

#ifndef AHL_DISCONNECT_POLICY

//#define AK_POLICY_DEMO  //For Audiokinetic demo only

typedef struct StreamPolicyInfo {
    streamID_t          streamID;
    int                 RolePriority;
    char *              pAudioRole;    
    InterruptBehaviorT  interruptBehavior;   
    int                 iDuckVolume;     //duck Volume
} StreamPolicyInfoT;
 
typedef struct EndPointPolicyInfo {
    endpointID_t    endpointID;
    EndpointTypeT   type;    
    DeviceURITypeT  deviceType;
    char *          pDeviceName;
    char *          pHalApiName; 
    int             iVolume;     //Current Volume            
    GArray *        streamInfo; //List of playing or duck stream at a given endpoint
} EndPointPolicyInfoT;

typedef enum SystemState {
    SYSTEM_STARTUP = 0,     // Startup
    SYSTEM_SHUTDOWN,        // ShutDown
    SYSTEM_NORMAL,          // Normal
    SYSTEM_LOW_POWER,       // Low Power, save mode
    SYSTEM_MAXVALUE         // Enum count, keep at the end
} SystemStateT;

typedef struct HalInfo {
    char *pDevID;
    char *pAPIName;
    char *pDisplayName;
} HalInfoT;

typedef struct StreamConfig {
    int iNbMaxStream;
    int iVolumeInit;
    int iVolumeDuckValue;
} StreamConfigT;

// Global Policy Local context
typedef struct PolicyLocalCtx {
    GArray *     pSourceEndpoints; // List of Source Endpoint with playing stream or interrupted stream
    GArray *     pSinkEndpoints;   // List of Sink Endpoint with playing stream or interrupted stream
    GPtrArray *  pHALList;
    SystemStateT systemState;
} PolicyLocalCtxT;

//  Policy context
PolicyLocalCtxT g_PolicyCtx;

//  Helper Functions
static int getStreamConfig(char *pAudioRole, StreamConfigT *pStreamConfig)
{
    if(pAudioRole == NULL || pStreamConfig==NULL)
    {
        return POLICY_FAIL;
    }

    if ( strcasecmp(pAudioRole,AHL_ROLE_WARNING) == 0 )
    {
        pStreamConfig->iNbMaxStream = 4;
        pStreamConfig->iVolumeInit = 80; 
        pStreamConfig->iVolumeDuckValue = 0;
    }
    else if ( strcasecmp(pAudioRole,AHL_ROLE_GUIDANCE) == 0 )
    {
        pStreamConfig->iNbMaxStream = 10;
        pStreamConfig->iVolumeInit = 70; 
        pStreamConfig->iVolumeDuckValue = 30;
    }
    else if ( strcasecmp(pAudioRole,AHL_ROLE_NOTIFICATION) == 0 )
    {
        pStreamConfig->iNbMaxStream = 4;
        pStreamConfig->iVolumeInit = 80; 
        pStreamConfig->iVolumeDuckValue = 10;
    }
    else if ( strcasecmp(pAudioRole,AHL_ROLE_COMMUNICATION) == 0 )
    {
        pStreamConfig->iNbMaxStream = 10;
        pStreamConfig->iVolumeInit = 70; 
        pStreamConfig->iVolumeDuckValue = 10;        
    }
    else if ( strcasecmp(pAudioRole,AHL_ROLE_ENTERTAINMENT) == 0 )
    {
        pStreamConfig->iNbMaxStream = MAX_ACTIVE_STREAM_POLICY;
        pStreamConfig->iVolumeInit = 60; 
        pStreamConfig->iVolumeDuckValue = 40;        
    }
    else if ( strcasecmp(pAudioRole,AHL_ROLE_SYSTEM) == 0 )
    {
        pStreamConfig->iNbMaxStream = 2;
        pStreamConfig->iVolumeInit = 100; 
        pStreamConfig->iVolumeDuckValue = 0;        
    }
    else if ( strcasecmp(pAudioRole,AHL_ROLE_STARTUP) == 0 )
    {
        pStreamConfig->iNbMaxStream = 1;
        pStreamConfig->iVolumeInit = 90; 
        pStreamConfig->iVolumeDuckValue = 0;        
    }
    else if ( strcasecmp(pAudioRole,AHL_ROLE_SHUTDOWN) == 0 )
    {
        pStreamConfig->iNbMaxStream = 1;
        pStreamConfig->iVolumeInit = 90; 
        pStreamConfig->iVolumeDuckValue = 0;        
    }
    return POLICY_SUCCESS;
}

static int PolicySetVolume(int iEndpointID, int iEndpointType, char *pHalApiName, char *AudioRole, DeviceURITypeT deviceType, int iVolume, bool bMute)
{
    if(pHalApiName == NULL || (strcasecmp(pHalApiName, AHL_POLICY_UNDEFINED_HALNAME) == 0))
    {
        AFB_WARNING("SetVolume cannot be accomplished without proper HAL association");    
        return POLICY_FAIL;
    }

    if(AudioRole == NULL)
    {
        AFB_ERROR("Invalid AudioRole : %s",AudioRole);    
        return POLICY_FAIL;
    }

    // Using audio role available from endpoint to target the right HAL control (build string based on convention)
    GString * gsHALControlName = NULL;    
    switch(deviceType)
    {
        case DEVICEURITYPE_ALSA_HW:
            gsHALControlName  = g_string_new("Master_Playback_Volume");    
            break;        
        case DEVICEURITYPE_ALSA_DMIX:
        case DEVICEURITYPE_ALSA_DSNOOP:
        case DEVICEURITYPE_ALSA_PLUG:
        case DEVICEURITYPE_ALSA_SOFTVOL:            
            gsHALControlName  = g_string_new(AudioRole);
            if(bMute == false)
            {
                AFB_DEBUG("Using ramp");
                g_string_append(gsHALControlName,"_Ramp");   
            }
            else
            {
                AFB_DEBUG("Not using ramp");
                g_string_append(gsHALControlName,"_Vol"); // no ramping     
            }                        
            break;
        default:
            // Not supported yet
            AFB_WARNING("Device Type %i is not support and can't set volume on HalName %s",deviceType, pHalApiName);
            return POLICY_FAIL;
            break;
    }

    // Set endpoint volume using HAL services (leveraging ramps etc.)
    json_object *j_response = NULL, *j_query = NULL;
 
    // Package query
    int err = wrap_json_pack(&j_query,"{s:s,s:i}","label",gsHALControlName->str, "val",iVolume);
    if (err) 
    {
        AFB_ERROR("Invalid query for HAL ctlset: %s with errorcode: %i",json_object_to_json_string(j_query), err);    
        return POLICY_FAIL;
    }

    // TODO: implement Volume limitation based on policy

    // Set the volume using the HAL
     err = afb_service_call_sync(pHalApiName, "ctlset", j_query, &j_response);
     if (err) 
     {
         AFB_ERROR("Could not ctlset on HAL: %s with errorcode: %i",pHalApiName, err);    
         return POLICY_FAIL;
     }
     AFB_DEBUG("HAL ctlset response=%s", json_object_to_json_string(j_response));

     if (bMute == false) {
        // Package event data
        json_object * eventDataJ = NULL;
        err = wrap_json_pack(&eventDataJ,"{s:s,s:i,s:i,s:i,s:s}","event_name", AHL_ENDPOINT_VOLUME_EVENT,"endpoint_id", iEndpointID, "endpoint_type", iEndpointType,"value",iVolume, "audio_role", AudioRole);
        if (err) 
        {
            AFB_ERROR("Invalid event data for volume event %s with errorcode: %i",json_object_to_json_string(eventDataJ), err);    
            return POLICY_FAIL;
        }

        audiohlapi_raise_event(eventDataJ);
     }

    return POLICY_SUCCESS; 
}

static int PolicyGetVolume(int iEndpointID, int iEndpointType, char *pHalApiName, char *AudioRole, DeviceURITypeT deviceType, int *pVolume)
{
    GString * gsHALControlName = NULL;

    // Using audio role available from endpoint to target the right HAL control (build string based on convention)        
    switch(deviceType)
    {
        case DEVICEURITYPE_ALSA_HW:
            gsHALControlName  = g_string_new("Master_Playback_Volume");    
            break;        
        case DEVICEURITYPE_ALSA_DMIX:
        case DEVICEURITYPE_ALSA_DSNOOP:
        case DEVICEURITYPE_ALSA_PLUG:
        case DEVICEURITYPE_ALSA_SOFTVOL:            
            gsHALControlName  = g_string_new(AudioRole);
            g_string_append(gsHALControlName,"_Vol"); // Return target value  
            break;
        default:
            // Set volume to zero for display purpose only.
            // Not supported yet
            *pVolume = 0;
            AFB_WARNING("Can't get volume on HAL: %s for device type: %d",pHalApiName,deviceType);
            return POLICY_FAIL;
    }

    // Set endpoint volume using HAL services (leveraging ramps etc.)
    json_object *j_response = NULL, *j_query = NULL;
 
    // Package query
    int err = wrap_json_pack(&j_query,"{s:s}","label",gsHALControlName->str);
    if (err) 
    {
        AFB_WARNING("Invalid query for HAL ctlset: %s, errorcode: %i",json_object_to_json_string(j_query),err);    
        return POLICY_FAIL;
    }

    // Get the volume using the HAL
     err = afb_service_call_sync(pHalApiName, "ctlget", j_query, &j_response);
    if (err) 
    {
         AFB_WARNING("Could not ctlset on HAL: %s, errorcode: %i",pHalApiName, err);    
         return POLICY_FAIL;
     }
    AFB_DEBUG("HAL ctlget response=%s", json_object_to_json_string(j_response));
 
    // Parse response
    json_object * jRespObj = NULL;
    json_object_object_get_ex(j_response, "response", &jRespObj);
    json_object * jVal = NULL;
    int val1 = 0, val2 = 0; // Why 2 values?    

    json_object_object_get_ex(jRespObj, "val", &jVal);
    int nbElement = json_object_array_length(jVal);
    if(nbElement == 2)
    {          
        err = wrap_json_unpack(jVal, "[ii]", &val1, &val2);
        if (err) {
            AFB_ERROR("Volume retrieve failed Could not retrieve volume value -> %s", json_object_get_string(jVal));
            return POLICY_FAIL;
        }  
    }
    else
    {    
        err = wrap_json_unpack(jVal, "[i]", &val1);
        if (err) {
            AFB_ERROR("Volume retrieve failed Could not retrieve volume value -> %s", json_object_get_string(jVal));
            return POLICY_FAIL;
        }
    }
        
   *pVolume = val1;

     // Package event data
    json_object * eventDataJ = NULL;
    err = wrap_json_pack(&eventDataJ,"{s:s,s:i,s:i,s:i,s:s}","event_name", AHL_ENDPOINT_VOLUME_EVENT,"endpoint_id", iEndpointID, "endpoint_type", iEndpointType,"value",*pVolume, "audio_role", AudioRole);
    if (err) 
    {
        AFB_ERROR("Invalid event data for volume event %s with errorcode: %i",json_object_to_json_string(eventDataJ), err);    
        return POLICY_FAIL;
    }

    audiohlapi_raise_event(eventDataJ);

    return POLICY_SUCCESS; 
}

static void PolicyPostStateEvent(int iStreamID, StreamEventT eventState)
{
    json_object * eventDataJ = NULL;
    int err = wrap_json_pack(&eventDataJ,"{s:s,s:i,s:i}","event_name", AHL_STREAM_STATE_EVENT,"stream_id",iStreamID, "state_event",eventState);
    if (err) 
    {
        AFB_ERROR("Invalid event data for stream state event: %s",json_object_to_json_string(eventDataJ));
    }
    else                    
    {
        audiohlapi_raise_event(eventDataJ);
    }         
}

static EndPointPolicyInfoT *PolicySearchEndPoint(EndpointTypeT type, char *pDeviceName)
{   
    GArray *pcurEndpointArray = NULL;

    if(type==ENDPOINTTYPE_SINK)
    {
        pcurEndpointArray = g_PolicyCtx.pSinkEndpoints;
    }
    else
    {
        pcurEndpointArray = g_PolicyCtx.pSourceEndpoints;
    }

    for(int i=0; i<pcurEndpointArray->len; i++)
    {
        EndPointPolicyInfoT * pCurEndpoint = &g_array_index(pcurEndpointArray,EndPointPolicyInfoT,i);    

        if(strcasecmp(pCurEndpoint->pDeviceName,pDeviceName)==0)
        {
            return pCurEndpoint;
        }
    }

    return NULL;
}

static int PolicyAddEndPoint(StreamInterfaceInfoT *pStreamInfo)
{
    EndPointPolicyInfoT *pPolicyEndPoint = PolicySearchEndPoint(pStreamInfo->endpoint.type, pStreamInfo->endpoint.gsDeviceName);
    if(pPolicyEndPoint == NULL)
    {
        //create EndPoint and add playing stream
        EndPointPolicyInfoT newEndPointPolicyInfo;        
        newEndPointPolicyInfo.endpointID  = pStreamInfo->endpoint.endpointID;
        newEndPointPolicyInfo.type        = pStreamInfo->endpoint.type;
        newEndPointPolicyInfo.deviceType  = pStreamInfo->endpoint.deviceURIType;
        newEndPointPolicyInfo.pDeviceName = strdup(pStreamInfo->endpoint.gsDeviceName);
        newEndPointPolicyInfo.pHalApiName = strdup(pStreamInfo->endpoint.gsHALAPIName);
        newEndPointPolicyInfo.iVolume = pStreamInfo->endpoint.iVolume;
        newEndPointPolicyInfo.streamInfo = g_array_new(FALSE, TRUE, sizeof(StreamPolicyInfoT));

        if(pStreamInfo->endpoint.type == ENDPOINTTYPE_SINK)
        {
            g_array_append_val(g_PolicyCtx.pSinkEndpoints, newEndPointPolicyInfo);
        }
        else
        {
            g_array_append_val(g_PolicyCtx.pSourceEndpoints, newEndPointPolicyInfo);
        }            
        
    }
    return POLICY_SUCCESS;    
}

static int PolicyAddStream(EndPointPolicyInfoT *pCurrEndPointPolicy, StreamInterfaceInfoT *pStreamInfo)
{
    StreamPolicyInfoT newStreamPolicyInfo;

    newStreamPolicyInfo.streamID = pStreamInfo->streamID;                    
    newStreamPolicyInfo.RolePriority = pStreamInfo->iPriority;                    
    newStreamPolicyInfo.pAudioRole = pStreamInfo->pRoleName;                    
    newStreamPolicyInfo.interruptBehavior = pStreamInfo->eInterruptBehavior;                    
    newStreamPolicyInfo.iDuckVolume = 0;                            
    g_array_append_val(pCurrEndPointPolicy->streamInfo, newStreamPolicyInfo);                    
    return POLICY_SUCCESS;    
}

static int PolicyRunningIdleTransition(EndPointPolicyInfoT *pCurrEndPointPolicy,StreamInterfaceInfoT * pStreamInfo)
{
    int err=0;
    if(pCurrEndPointPolicy == NULL || pCurrEndPointPolicy->streamInfo->len == 0)
    {
        AFB_ERROR("StreamID not found in active endpoint when running to idle transition is requested");
        return POLICY_FAIL; 
    }
    // Search for the matching stream
    for(int i=0; i<pCurrEndPointPolicy->streamInfo->len; i++)
    {
        StreamPolicyInfoT currentPolicyStreamInfo = g_array_index(pCurrEndPointPolicy->streamInfo,StreamPolicyInfoT,i);
        if(currentPolicyStreamInfo.streamID == pStreamInfo->streamID)
        {
            //remove the current stream
            g_array_remove_index(pCurrEndPointPolicy->streamInfo, i);            
            if(pCurrEndPointPolicy->streamInfo->len > 0) //need to unduck
            {
                //check the last element(Akways highest priority)
                StreamPolicyInfoT HighPriorityStreamInfo = g_array_index(pCurrEndPointPolicy->streamInfo,StreamPolicyInfoT,pCurrEndPointPolicy->streamInfo->len-1);
                switch(currentPolicyStreamInfo.interruptBehavior)
                {
                    case INTERRUPTBEHAVIOR_CONTINUE:                                    
                        //unduck and set Volume back to original value
                        err = PolicySetVolume(pCurrEndPointPolicy->endpointID, 
                                             pCurrEndPointPolicy->type,
                                             pCurrEndPointPolicy->pHalApiName, 
                                             HighPriorityStreamInfo.pAudioRole, 
                                             pCurrEndPointPolicy->deviceType, 
                                             HighPriorityStreamInfo.iDuckVolume,
                                             false);
                        if(err)                
                        {                                
                            return POLICY_FAIL; 
                        }
                        
                        return POLICY_SUCCESS;                                            
                    case INTERRUPTBEHAVIOR_PAUSE:
                        PolicyPostStateEvent(HighPriorityStreamInfo.streamID,STREAM_EVENT_RESUME);                        
                        return POLICY_SUCCESS;                                            
                    case INTERRUPTBEHAVIOR_CANCEL:
                        PolicyPostStateEvent(HighPriorityStreamInfo.streamID,STREAM_EVENT_START);                        
                        return POLICY_SUCCESS;                                            
                    default:
                        AFB_ERROR("Unsupported Intterupt Behavior");
                        return POLICY_FAIL; 
                } 
            }            
        }
    }      
    return POLICY_SUCCESS;
}

static int PolicyIdleRunningTransition(EndPointPolicyInfoT *pCurrEndPointPolicy, StreamInterfaceInfoT * pStreamInfo)
{
    int err=0;    
    if(pCurrEndPointPolicy->streamInfo == NULL)
    {
        AFB_ERROR("pCurrEndPointPolicy->streamInfo is null on an active endpoint");
        return POLICY_FAIL;        
    }

    if(pCurrEndPointPolicy->streamInfo->len == 0) //No stream is playing on this endpoint
    {                           
        PolicyAddStream(pCurrEndPointPolicy, pStreamInfo);
    }
    else //Interrupt case
    {
        //check the last element 
        StreamPolicyInfoT *pCurrentActiveStreamInfo = &g_array_index(pCurrEndPointPolicy->streamInfo,StreamPolicyInfoT,pCurrEndPointPolicy->streamInfo->len-1);
        g_assert_nonnull(pCurrentActiveStreamInfo);
        if(pStreamInfo->iPriority >= pCurrentActiveStreamInfo->RolePriority)
        {
            switch(pStreamInfo->eInterruptBehavior)
            {
                case INTERRUPTBEHAVIOR_CONTINUE:
                    //Save the current Volume and set the docking volume
                    pCurrentActiveStreamInfo->iDuckVolume = pStreamInfo->endpoint.iVolume;
                    StreamConfigT StreamConfig;
                    err = getStreamConfig(pStreamInfo->pRoleName, &StreamConfig);
                    if(err == POLICY_FAIL)
                    {
                        AFB_ERROR("Error getting stream configuration for audiorole: %s", pStreamInfo->pRoleName);                        
                        return POLICY_FAIL; 
                    }
                    err = PolicySetVolume(pCurrEndPointPolicy->endpointID, 
                                         pCurrEndPointPolicy->type,
                                         pCurrEndPointPolicy->pHalApiName, 
                                         pCurrentActiveStreamInfo->pAudioRole, 
                                         pCurrEndPointPolicy->deviceType, 
                                         StreamConfig.iVolumeDuckValue,
                                         false);
                    if(err)                
                    {
                        AFB_ERROR("Set Volume return with errorcode%i for streamID: %i and Hal:%s", err, pCurrentActiveStreamInfo->streamID, pCurrEndPointPolicy->pHalApiName);                        
                        return POLICY_FAIL; 
                    }                                            
                    break;
                case INTERRUPTBEHAVIOR_PAUSE:
                    PolicyPostStateEvent(pCurrentActiveStreamInfo->streamID,STREAM_EVENT_PAUSE);                        
                    break;
                case INTERRUPTBEHAVIOR_CANCEL:
                    PolicyPostStateEvent(pCurrentActiveStreamInfo->streamID,STREAM_EVENT_STOP);                        
                    g_array_remove_index(pCurrEndPointPolicy->streamInfo, pCurrEndPointPolicy->streamInfo->len-1);
                    break;
                default:
                    AFB_ERROR("Unsupported Intterupt Behavior");
                    return AHL_POLICY_REJECT; 
            } 

            //Add the playing stream at last
            PolicyAddStream(pCurrEndPointPolicy, pStreamInfo);
        }
        else
        {
            //Higher Priority Stream is playing
            AFB_ERROR("Higher Priority Stream is playing");
            return POLICY_FAIL; 
        }                
    }

    return POLICY_SUCCESS;
}

static void PolicySpeedModify(int speed)
{
    for(int i=0; i<g_PolicyCtx.pSinkEndpoints->len; i++)
    {
        EndPointPolicyInfoT * pCurEndpoint = &g_array_index(g_PolicyCtx.pSinkEndpoints,EndPointPolicyInfoT,i);    
        if(pCurEndpoint == NULL)
        {
            AFB_WARNING("Sink Endpoint not found");
            return;
        }

        //check if active 
        if(pCurEndpoint->streamInfo->len > 0 )
        {
            StreamPolicyInfoT * pCurStream = &g_array_index(pCurEndpoint->streamInfo,StreamPolicyInfoT,pCurEndpoint->streamInfo->len-1);            
            if(strcasecmp(pCurStream->pAudioRole,AHL_ROLE_ENTERTAINMENT)==0)
            {
                if(speed > 30 && speed < 100)
                {
                    int volume =speed;
                    PolicySetVolume(pCurEndpoint->endpointID,
                                    pCurEndpoint->type,                                    
                                    pCurEndpoint->pHalApiName,
                                    pCurStream->pAudioRole,
                                    pCurEndpoint->deviceType,                                    
                                    volume,
                                    false); 
                }                
            }            
        }
    }
}

static int RetrieveAssociatedHALAPIName(int iAlsaCardNumber,char ** out_pDisplayName,char ** out_pHALName)
{
    *out_pDisplayName = NULL;
    *out_pHALName = NULL;

    if(g_PolicyCtx.pHALList)
    {
        for(int i=0; i<g_PolicyCtx.pHALList->len; i++)
        {
            HalInfoT *pHalInfo = g_ptr_array_index(g_PolicyCtx.pHALList, i);
            // Retrieve card number (e.g. hw:0)
            int iCardNum = atoi(pHalInfo->pDevID+3);
            if (iCardNum == iAlsaCardNumber) {
                *out_pDisplayName = pHalInfo->pDisplayName;
                *out_pHALName = pHalInfo->pAPIName;
                return POLICY_SUCCESS;
            }            
        }
    }
    
    return POLICY_FAIL;
}

static int GetHALList(void)
{
    json_object *j_response = NULL, *j_query = NULL;
    int err = afb_service_call_sync("alsacore", "hallist", j_query, &j_response);
    if (err) {
        AFB_ERROR("Could not retrieve list of HAL from ALSA core");
        return POLICY_FAIL;
    }
    AFB_DEBUG("ALSAcore hallist response=%s", json_object_to_json_string(j_response));

    // Look through returned list for matching card
    json_object * jRespObj = NULL;
    json_object_object_get_ex(j_response, "response", &jRespObj);
    int iNumHAL = json_object_array_length(jRespObj);
    for ( int i = 0 ; i < iNumHAL; i++)
    {
        json_object * jHAL = json_object_array_get_idx(jRespObj,i);
        char * pDevIDStr = NULL;
        char * pAPIName = NULL;
        char * pShortName = NULL;
        
        int err = wrap_json_unpack(jHAL, "{s:s,s:s,s:s}", "devid", &pDevIDStr,"api", &pAPIName,"shortname",&pShortName);
        if (err) {
            AFB_ERROR("Could not retrieve devid string=%s", json_object_get_string(jHAL));
            return POLICY_FAIL;
        }

        HalInfoT *pHalInfo = (HalInfoT*)malloc(sizeof(HalInfoT));
        if(pHalInfo == NULL)
        {
            AFB_ERROR("Unable to allocate memory for HalInfo");
            return POLICY_FAIL;
        }

        pHalInfo->pDevID = strdup(pDevIDStr);
        pHalInfo->pAPIName = strdup(pAPIName);
        pHalInfo->pDisplayName = strdup(pShortName);

        g_ptr_array_add( g_PolicyCtx.pHALList, pHalInfo);
    }

    return POLICY_SUCCESS;
}

//
//  Policy API Functions
//
int Policy_OpenStream(json_object *pStreamJ)
{
    StreamInterfaceInfoT Stream;    

    int err = JSONToStream(pStreamJ, &Stream);
    if(err == AHL_POLICY_UTIL_FAIL)
    {
        return AHL_POLICY_REJECT;     
    }

    // Example rule -> when system is in shutdown or low power mode, no audio stream can be opened (return AHL_POLICY_REJECT)
    // Should receive event from lower level layer
    if(g_PolicyCtx.systemState != SYSTEM_NORMAL)
    {
        return AHL_POLICY_REJECT;
    } 
  
    StreamConfigT StreamConfig;
    err = getStreamConfig(Stream.pRoleName, &StreamConfig);
    if(err == POLICY_FAIL)
    {
        return AHL_POLICY_REJECT;     
    }

    // Volume support only possible through ALSA
    if(Stream.endpoint.deviceURIType != DEVICEURITYPE_NOT_ALSA) {
        err = PolicyGetVolume(Stream.endpoint.endpointID, 
                              Stream.endpoint.type,
                              Stream.endpoint.gsHALAPIName, 
                              Stream.endpoint.pRoleName, 
                              Stream.endpoint.deviceURIType, 
                              &Stream.endpoint.iVolume);
        if(err == POLICY_FAIL)
        {
            return AHL_POLICY_REJECT;
        }
    }

    err = PolicyAddEndPoint(&Stream);
    if(err == POLICY_FAIL)
    {
        return AHL_POLICY_REJECT;
    }
    return AHL_POLICY_ACCEPT; 
}

int Policy_CloseStream(json_object *pStreamJ)
{
    StreamInterfaceInfoT Stream;    
    int err = JSONToStream(pStreamJ, &Stream);
    if(err == AHL_POLICY_UTIL_FAIL)
    {
        return AHL_POLICY_REJECT;     
    }

    return AHL_POLICY_ACCEPT; 
}

int  Policy_SetStreamState(json_object *pStreamJ)
{  
    // Optional TODO: Could mute endpoint before activation, unmute afterwards (after a delay?) to avoid noises

    StreamStateT streamState = 0;
    StreamInterfaceInfoT Stream;

    int err = JSONToStream(pStreamJ, &Stream);
    if(err == AHL_POLICY_UTIL_FAIL)
    {
        return AHL_POLICY_REJECT;     
    }

    json_object *streamStateJ = NULL;

    if(!json_object_object_get_ex(pStreamJ, "arg_stream_state", &streamStateJ))
    {
        return AHL_POLICY_REJECT;     
    }
    streamState = (StreamStateT)json_object_get_int(streamStateJ);

    // Change of state
    if(Stream.streamState != streamState)
    {
        //seach corresponding endpoint and gather information on it        
        EndPointPolicyInfoT *pCurrEndPointPolicy = PolicySearchEndPoint(Stream.endpoint.type , Stream.endpoint.gsDeviceName);
    
        switch(Stream.streamState)
        {
            case STREAM_STATE_IDLE:            
                switch(streamState)
                {
                    case STREAM_STATE_RUNNING:
                        err = PolicyIdleRunningTransition(pCurrEndPointPolicy, &Stream);
                        if(err)
                        {
                            return AHL_POLICY_REJECT;  
                        }                        
                        PolicyPostStateEvent(Stream.streamID,STREAM_EVENT_START); 
                        break;
                    case STREAM_STATE_PAUSED:
                        err = PolicyIdleRunningTransition(pCurrEndPointPolicy, &Stream);
                        if(err)
                        {
                            return AHL_POLICY_REJECT;  
                        }  
                        PolicyPostStateEvent(Stream.streamID,STREAM_EVENT_PAUSE); 
                        break;
                    default:
                        return AHL_POLICY_REJECT;  
                }
                break;
            case STREAM_STATE_RUNNING:    
                switch(streamState)
                {
                    case STREAM_STATE_IDLE:
                        err = PolicyRunningIdleTransition(pCurrEndPointPolicy, &Stream);
                        if(err)
                        {
                            return AHL_POLICY_REJECT;  
                        }                                            
                        PolicyPostStateEvent(Stream.streamID,STREAM_EVENT_STOP);                                       
                        break;
                    case STREAM_STATE_PAUSED:
                        PolicyPostStateEvent(Stream.streamID,STREAM_EVENT_PAUSE);                                       
                        break;
                    default:
                        return AHL_POLICY_REJECT;  
                }
                break;
            case STREAM_STATE_PAUSED:    
                switch(streamState)
                {
                    case STREAM_STATE_IDLE:
                        err = PolicyRunningIdleTransition(pCurrEndPointPolicy, &Stream);
                        if(err)
                        {
                            return AHL_POLICY_REJECT;  
                        }                        
                        PolicyPostStateEvent(Stream.streamID,STREAM_EVENT_STOP);    
                        break;
                    case STREAM_STATE_RUNNING:
                        PolicyPostStateEvent(Stream.streamID,STREAM_EVENT_RESUME); 
                        break;                 
                    default:
                        return AHL_POLICY_REJECT;  
                }
                break;
            default:
                return AHL_POLICY_REJECT;  
        }      
    }
    return AHL_POLICY_ACCEPT; 
}

int  Policy_SetStreamMute(json_object *pStreamJ)
{
    // Note: stream mute currently implemented directly using ALSA volume. It should later be implemented with a distinct mute switch control instead
    StreamMuteT streamMute = 0;
    StreamInterfaceInfoT Stream;    

    int err = JSONToStream(pStreamJ, &Stream);
    if(err == AHL_POLICY_UTIL_FAIL)
    {
        return AHL_POLICY_REJECT;     
    }

    json_object *streamMuteJ=NULL;

    if(!json_object_object_get_ex(pStreamJ, "mute_state", &streamMuteJ))
    {
        return AHL_POLICY_REJECT;     
    }
    streamMute = (StreamMuteT)json_object_get_int(streamMuteJ);

    if(streamMute != Stream.streamMute)
    {
        if(streamMute == STREAM_MUTED)
        {
            err = PolicySetVolume(Stream.endpoint.endpointID, 
                                Stream.endpoint.type,
                                Stream.endpoint.gsHALAPIName,
                                Stream.pRoleName,
                                Stream.endpoint.deviceURIType,
                                0,     // mute volume                                 
                                true); // no ramp and no volume event
            if(err)                
            {
                AFB_ERROR("StreamID:%i Set Volume return with errorcode%i",Stream.streamID, err);                        
                return AHL_POLICY_REJECT;  
            }   
            PolicyPostStateEvent(Stream.streamID,STREAM_EVENT_MUTED); 
        }
        else
        {
            err = PolicySetVolume(Stream.endpoint.endpointID,
                                Stream.endpoint.type,
                                Stream.endpoint.gsHALAPIName,
                                Stream.pRoleName,
                                Stream.endpoint.deviceURIType,                                    
                                Stream.endpoint.iVolume, // restore volume
                                true); // no ramp and no volume event 
            if(err)                
            {
                AFB_ERROR("Endpoint:%i Set Volume return with errorcode%i",Stream.streamID, err);                        
                return AHL_POLICY_REJECT;  
            }   
            PolicyPostStateEvent(Stream.streamID,STREAM_EVENT_UNMUTED); 
        }
    }
    
    return AHL_POLICY_ACCEPT;
}

int Policy_SetVolume(json_object *pEndpointJ)
{
    char *volumeStr = NULL;
    EndPointInterfaceInfoT Endpoint;
  
    int err = JSONToEndpoint(pEndpointJ, &Endpoint);
    if(err == AHL_POLICY_UTIL_FAIL)
    {
        return AHL_POLICY_REJECT;     
    }

    json_object *volumeJ = NULL;
    if(!json_object_object_get_ex(pEndpointJ, "arg_volume", &volumeJ))
    {
        return AHL_POLICY_REJECT;     
    }
    volumeStr = (char*)json_object_get_string(volumeJ);

    // TODO: Parse volume string to support increment/absolute/percent notation
    int vol = atoi(volumeStr);
    
    // Set the volume
    err = PolicySetVolume(Endpoint.endpointID, 
                          Endpoint.type,
                          Endpoint.gsHALAPIName,
                          Endpoint.pRoleName,
                          Endpoint.deviceURIType,                            
                          vol,
                          false);  // Volume ramp and send events  
    if (err) 
    {
        AFB_ERROR("Set Volume return with errorcode %i", err);    
        return AHL_POLICY_REJECT;
    }

    return AHL_POLICY_ACCEPT; 
}

int Policy_SetProperty(json_object *pEndpointJ)
{
    char *propertyName = NULL;
    EndPointInterfaceInfoT Endpoint;

    int err = JSONToEndpoint(pEndpointJ, &Endpoint);
    if(err == AHL_POLICY_UTIL_FAIL)
    {
        return AHL_POLICY_REJECT;     
    }

    json_object *propertyNameJ = NULL;
    if(!json_object_object_get_ex(pEndpointJ, "arg_property_name", &propertyNameJ))
    {
        return AHL_POLICY_REJECT;     
    }
    propertyName = (char*)json_object_get_string(propertyNameJ);

    json_object *propValueJ = NULL;
    if(!json_object_object_get_ex(pEndpointJ, "arg_property_value", &propValueJ))
    {
        return AHL_POLICY_REJECT;     
    }
    json_type currentTypeJ = json_object_get_type(propValueJ);

    json_object *propArray = NULL;
    if(!json_object_object_get_ex(pEndpointJ, "properties", &propArray))
    {
        return AHL_POLICY_REJECT;     
    }

    int iPropArrayLen = json_object_array_length(propArray);        
    int foundProperty = 0;
    
    for (int i = 0; i < iPropArrayLen; i++) 
    {
        // get the i-th object in medi_array
        json_object *propElementJ = json_object_array_get_idx(propArray, i);
        if(propElementJ)
        {
            json_object *propElementNameJ=NULL;
            if(json_object_object_get_ex(propElementJ, "property_name", &propElementNameJ))
            {
                char *propElementName = (char*)json_object_get_string(propElementNameJ);   
                if(strcasecmp(propElementName,propertyName)==0)         
                {
                    json_object *propElementValueJ=NULL;
                    if(!json_object_object_get_ex(propElementJ, "property_value", &propElementValueJ))
                    {
                        json_type elementTypeJ = json_object_get_type(propElementValueJ);

                        // Apply policy on set property if needed here
                        // Here we only validate that the type is the same
                        if(currentTypeJ != elementTypeJ)
                        {
                            AFB_ERROR("Property Value Type is wrong");
                            return AHL_POLICY_REJECT; 
                        }
                        foundProperty = 1;
                        break;   
                    }
                }
            }                        
        }
    }

    if(foundProperty== 0)
    {
        AFB_ERROR("Can't find property %s, request will be rejected", propertyName);
        return AHL_POLICY_REJECT;  
    }

    // Create a new Json Object    
    json_object *pEventDataJ = NULL;
    err = wrap_json_pack(&pEventDataJ,"{s:s,s:i,s:i,s:s,s:o,s:s}",
                        "event_name", AHL_ENDPOINT_PROPERTY_EVENT,
                        "endpoint_id", Endpoint.endpointID, 
                        "endpoint_type", Endpoint.type,
                        "property_name", propertyName,
                        "value",propValueJ, 
                        "audio_role", Endpoint.pRoleName);
    if(err)
    {
        AFB_ERROR("Unable to pack property event");        
        return AHL_POLICY_REJECT;
    }

    // Raise Event to update HLB
    audiohlapi_raise_event(pEventDataJ);

    return AHL_POLICY_ACCEPT;
}

int Policy_PostAction(json_object *pPolicyActionJ)
{
    char * actionName = NULL;  
    char * audioRole = NULL;
    char * mediaName = NULL;
    json_object *actionContext = NULL;

    int err = wrap_json_unpack(pPolicyActionJ, "{s:s,s:s,s?s,s?o}", "action_name", &actionName,"audio_role",&audioRole,"media_name",&mediaName,"action_context",&actionContext);
    if (err) {
        AFB_ERROR("Unable to pack JSON endpoint, =%s", wrap_json_get_error_string(err));
        return AHL_POLICY_REJECT;
    }
    
    // TODO: Any event with media specified should trigger action on provided rendering services (e.g. Wwise binding, gstreamer file player wrapper, MPDC? simple ALSA player (aplay)?)
    // Example (when the policy is hooked to CAN events). Post audio playback events other than safety during reverse gear engaged declined
    // Example post HMI audio role playback events declined when higher priority streams are active
    
    //In this use case just return the action back to highlevel binding.

    json_object *pEventDataJ = NULL;
    err = wrap_json_pack(&pEventDataJ, "{s:s,s:s,s:s,s?s,s?o}", "event_name", AHL_POST_ACTION_EVENT, "action_name", &actionName,"audio_role",&audioRole,"media_name",&mediaName,"action_context",&actionContext);
    if (err) {
        AFB_ERROR("Unable to pack JSON endpoint, =%s", wrap_json_get_error_string(err));
        return AHL_POLICY_REJECT;
    }

    audiohlapi_raise_event(pEventDataJ);

    return AHL_POLICY_ACCEPT;
}

int Policy_Endpoint_Init(json_object *pInPolicyEndpointJ,json_object **pOutPolicyEndpointJ)
{
    endpointID_t endpointID = AHL_UNDEFINED;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    DeviceURITypeT deviceURIType = DEVICEURITYPE_MAXVALUE;
    int iAllocString = 0;
    char * pRoleName = NULL;
    int iAlsaCardNumber = AHL_UNDEFINED;
    char * pDeviceName = NULL;
    int err = wrap_json_unpack(pInPolicyEndpointJ,"{s:i,s:i,s:i,s:s,s:i,s:s}",
                            "endpoint_id",&endpointID,
                            "endpoint_type",&endpointType,
                            "device_uri_type",&deviceURIType,
                            "audio_role",&pRoleName,
                            "alsa_cardNum", &iAlsaCardNumber,
                            "device_name", &pDeviceName ); 
    if (err) {
        AFB_ERROR("Unable to unpack JSON endpoint, =%s", wrap_json_get_error_string(err));
        goto OnError;          
    }

    StreamConfigT StreamConfig;
    getStreamConfig(pRoleName, &StreamConfig);

    char * pDisplayName = NULL;
    char * pHALName = NULL;
    if (deviceURIType != DEVICEURITYPE_NOT_ALSA) {
        // Update Hal Name
        err = RetrieveAssociatedHALAPIName(iAlsaCardNumber,&pDisplayName,&pHALName);
        if (err) {
            AFB_WARNING("HAL not found for device %s", pDeviceName);            
            pDisplayName = g_strdup(AHL_POLICY_UNDEFINED_DISPLAYNAME);
            pHALName = g_strdup(AHL_POLICY_UNDEFINED_HALNAME);
            iAllocString = 1;
        }
        else
        {
            // Set initial Volume
            err = PolicySetVolume(endpointID, 
                                endpointType,
                                pHALName, 
                                pRoleName, 
                                deviceURIType, 
                                StreamConfig.iVolumeInit,
                                true); // Do not raise event and no volume ramp
            if(err) {  
                goto OnError;                            
            }
        }
    }
    else {
        // Set display / HAL for non ALSA devices (default)
        pDisplayName = g_strdup(AHL_POLICY_UNDEFINED_DISPLAYNAME);
        pHALName = g_strdup(AHL_POLICY_UNDEFINED_HALNAME);
        iAllocString = 1;
    }

    // Populate special device property (TODO: Should be obtained from HAL)
    // if (strcasecmp(gsHALAPIName,"Device")==0)
    // {
            // Create json object for PropTable
            json_object *pPropTableJ = json_object_new_array();
            Add_Endpoint_Property_Int(pPropTableJ,AHL_PROPERTY_EQ_LOW,3);
            Add_Endpoint_Property_Int(pPropTableJ,AHL_PROPERTY_EQ_MID,0);
            Add_Endpoint_Property_Int(pPropTableJ,AHL_PROPERTY_EQ_HIGH,6);
            Add_Endpoint_Property_Int(pPropTableJ,AHL_PROPERTY_BALANCE,0);
            Add_Endpoint_Property_Int(pPropTableJ,AHL_PROPERTY_FADE,30);
            Add_Endpoint_Property_String(pPropTableJ,"preset_name","flat");
    // }
 
    err = wrap_json_pack(pOutPolicyEndpointJ,"{s:i,s:s,s:s,s:o}",
                    "init_volume",StreamConfig.iVolumeInit,
                    "display_name",pDisplayName,
                    "hal_name",pHALName,
                    "property_table",pPropTableJ
                    );
    if (err) {
        AFB_ERROR("Unable to pack JSON endpoint, =%s", wrap_json_get_error_string(err));
        goto OnError;                                    
    }
    
    // TODO: Future policy binding to return request response with pOutPolicyEndpointJ (instead of output argument)
    return AHL_POLICY_ACCEPT; // No errors

OnError:
    if (iAllocString) {
        g_free(pDisplayName);
        g_free(pHALName);
    }
    return AHL_POLICY_REJECT;

}

int Policy_Init()
{
    // Start fresh
    memset(&g_PolicyCtx,0,sizeof(g_PolicyCtx));

    // Initialize Ressources
    g_PolicyCtx.pSourceEndpoints = g_array_new(FALSE,TRUE,sizeof(EndPointPolicyInfoT));
    g_PolicyCtx.pSinkEndpoints = g_array_new(FALSE,TRUE,sizeof(EndPointPolicyInfoT));    
    g_PolicyCtx.pHALList = g_ptr_array_new_with_free_func(g_free);

    //Require AlsaCore Dependency
    int err = afb_daemon_require_api_v2(AHL_POLICY_ALSA_API,1) ;
    if( err != 0 )
    {
        AFB_ERROR("Audio Policy could not set dependency on alsacore API");
        return AHL_POLICY_REJECT;
    }

    // Get HalList
    GetHALList();

    // TODO: Register events from low level / HAL for dynamic changes

    // Set System Normal for now, this should be set by an event 
    g_PolicyCtx.systemState = SYSTEM_NORMAL;



#ifdef AK_POLICY_DEMO
    // Register audio backend events (TODO: should instead do this with signal composer with appropriate dependency)
    // This is to simulate can bus, only used for demo
    json_object *queryurl = NULL, *responseJ = NULL, *eventsJ = NULL;
    eventsJ = json_object_new_array();
    json_object_array_add(eventsJ, json_object_new_string("audiod_system_event"));
    queryurl = json_object_new_object();
    json_object_object_add(queryurl, "events", eventsJ);
    int returnResult = afb_service_call_sync("audiod", "subscribe", queryurl, &responseJ);
    if (returnResult) {
        AFB_ERROR("Fail subscribing to Audio Backend System events");
        return AHL_POLICY_REJECT;
    }
#endif


    return AHL_POLICY_ACCEPT;
}
 
void Policy_Term()
{
    // Free Ressources
    if (g_PolicyCtx.pHALList) {
        g_ptr_array_free(g_PolicyCtx.pHALList,TRUE);
        g_PolicyCtx.pHALList = NULL;
    }

    for(int i=0; i<g_PolicyCtx.pSourceEndpoints->len; i++)
    {
        EndPointPolicyInfoT * pCurEndpoint = &g_array_index(g_PolicyCtx.pSourceEndpoints,EndPointPolicyInfoT,i);    
        g_array_free(pCurEndpoint->streamInfo,TRUE);
        pCurEndpoint->streamInfo= NULL;
    }

    for(int i=0; i<g_PolicyCtx.pSinkEndpoints->len; i++)
    {
        EndPointPolicyInfoT * pCurEndpoint = &g_array_index(g_PolicyCtx.pSinkEndpoints,EndPointPolicyInfoT,i);    
        g_array_free(pCurEndpoint->streamInfo,TRUE);
        pCurEndpoint->streamInfo = NULL;
    }
    
    g_array_free(g_PolicyCtx.pSourceEndpoints,TRUE);
    g_PolicyCtx.pSourceEndpoints = NULL;
    g_array_free(g_PolicyCtx.pSinkEndpoints,TRUE);
    g_PolicyCtx.pSinkEndpoints = NULL;
}

// For demo purpose only, should be listening to signal composer / CAN events instead
void Policy_OnEvent(const char *evtname, json_object *eventJ)
{
    AFB_DEBUG("Policy received event %s", evtname);
	char *eventName = NULL;
	json_object *event_parameter = NULL;
    int speed = 0;

    if(strcasecmp(evtname, "audiod/system_events")==0)
    {
        int err = wrap_json_unpack(eventJ, "{s:s,s:o}", "event_name", &eventName, "event_parameter", &event_parameter);
        if (err) {        
            AFB_WARNING("Invalid arguments, Args not a valid json object query=%s", json_object_get_string(eventJ));
            return;
        }
    
        if(strcasecmp(eventName, "speed")==0)
        {
            AFB_NOTICE("Invalid arguments, Args not a valid json object query=%s", json_object_get_string(event_parameter));
            err = wrap_json_unpack(event_parameter, "{s:i}", "speed_value", &speed);
            if (err) {        
                AFB_WARNING("Invalid arguments, Args not a valid json object query=%s", json_object_get_string(event_parameter));
                return;
            }   
            // When speed change Modify volume on Endpoint where entertainment change
            PolicySpeedModify(speed);
        }
    }
}

#endif // AHL_DISCONNECT_POLICY