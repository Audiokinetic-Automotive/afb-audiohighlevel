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

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "wrap-json.h"
#include "ahl-policy-utils.h"
#include "ahl-interface.h"
#include "ahl-policy.h"

#ifndef AHL_DISCONNECT_POLICY

#define MAX_ACTIVE_STREAM_POLICY 30
#define POLICY_FAIL     1
#define POLICY_SUCCESS  0

#define AHL_POLICY_UNDEFINED_HALNAME "UNDEFINED"
#define AHL_POLICY_UNDEFINED_DISPLAYNAME "DeviceNotFound"

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

//  Global Context
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
    if(pHalApiName == NULL)
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
            //Set volume to zero for display purpose only.
            //Not support yet
            AFB_WARNING("Device Type %i is not support and can't set volume on HalName %s",deviceType, pHalApiName);
            return POLICY_FAIL;
            break;
    }

    // Set endpoint volume using HAL services (leveraging ramps etc.)
    json_object *j_response, *j_query = NULL;
 
    // Package query
    int err = wrap_json_pack(&j_query,"{s:s,s:i}","label",gsHALControlName->str, "val",iVolume);
    if (err) 
    {
        AFB_ERROR("Invalid query for HAL ctlset: %s with errorcode: %i",json_object_to_json_string(j_query), err);    
        return POLICY_FAIL;
    }

    //TODO implement Volume limitation based on policy

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
            g_string_append(gsHALControlName,"_Vol"); // Or _Vol for direct control (no ramping)    
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

    //TODO implement Volume limitation based on policy

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


static int PolicyAddEndPoint(StreamInfoT *pStreamInfo)
{
    EndPointPolicyInfoT *pPolicyEndPoint = PolicySearchEndPoint(pStreamInfo->pEndpointInfo->type, pStreamInfo->pEndpointInfo->gsDeviceName);
    if(pPolicyEndPoint == NULL)
    {
        //create EndPoint and add playing stream
        EndPointPolicyInfoT newEndPointPolicyInfo;        
        newEndPointPolicyInfo.endpointID  = pStreamInfo->pEndpointInfo->endpointID;
        newEndPointPolicyInfo.type        = pStreamInfo->pEndpointInfo->type;
        newEndPointPolicyInfo.deviceType  = pStreamInfo->pEndpointInfo->deviceURIType;
        newEndPointPolicyInfo.pDeviceName = strdup(pStreamInfo->pEndpointInfo->gsDeviceName);
        newEndPointPolicyInfo.pHalApiName = strdup(pStreamInfo->pEndpointInfo->gsHALAPIName);
        newEndPointPolicyInfo.iVolume = pStreamInfo->pEndpointInfo->iVolume;
        newEndPointPolicyInfo.streamInfo = g_array_new(FALSE, TRUE, sizeof(StreamPolicyInfoT));;

        if(pStreamInfo->pEndpointInfo->type == ENDPOINTTYPE_SINK)
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


static int PolicyAddStream(EndPointPolicyInfoT *pCurrEndPointPolicy, StreamInfoT *pStreamInfo)
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

static int PolicyRunningIdleTransition(EndPointPolicyInfoT *pCurrEndPointPolicy,StreamInfoT * pStreamInfo)
{
    int err=0;
    if(pCurrEndPointPolicy == NULL || pCurrEndPointPolicy->streamInfo->len == 0)
    {
        //Remove endpoint
        AFB_ERROR("StreamID not found in active Endpoint when Running to Idle transition is request");
        return POLICY_FAIL; 
    }
    //Search for the matching stream
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
                        err= PolicySetVolume(pCurrEndPointPolicy->endpointID, 
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
                        break;
                    case INTERRUPTBEHAVIOR_PAUSE:
                        //pInterruptStreamInfo->streamState = STREAM_STATE_RUNNING;
                        PolicyPostStateEvent(HighPriorityStreamInfo.streamID,STREAM_EVENT_RESUME);                        
                        return POLICY_SUCCESS;                                            
                        break;

                    case INTERRUPTBEHAVIOR_CANCEL:
                        PolicyPostStateEvent(HighPriorityStreamInfo.streamID,STREAM_EVENT_START);                        
                        return POLICY_SUCCESS;                                            
                        break;
                    default:
                        AFB_ERROR("Unsupported Intterupt Behavior");
                        return POLICY_FAIL; 
                        break;
                } 
            }            
        }
    }      
    return POLICY_SUCCESS;
}

static int PolicyIdleRunningTransition(EndPointPolicyInfoT *pCurrEndPointPolicy, StreamInfoT * pStreamInfo)
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
                    pCurrentActiveStreamInfo->iDuckVolume = pStreamInfo->pEndpointInfo->iVolume;
                    StreamConfigT StreamConfig;
                    err = getStreamConfig(pStreamInfo->pRoleName, &StreamConfig);
                    if(err == POLICY_FAIL)
                    {
                        AFB_ERROR("Error getting stream configuration for audiorole: %s", pStreamInfo->pRoleName);                        
                        return POLICY_FAIL; 
                    }
                    err= PolicySetVolume(pCurrEndPointPolicy->endpointID, 
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
                    break;

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

static int RetrieveAssociatedHALAPIName(EndpointInfoT * io_pEndpointInfo)
{
    if(g_PolicyCtx.pHALList)
    {
        for(int i=0; i<g_PolicyCtx.pHALList->len; i++)
        {
            HalInfoT *pHalInfo = g_ptr_array_index(g_PolicyCtx.pHALList, i);
            // Retrieve card number (e.g. hw:0)
            int iCardNum = atoi(pHalInfo->pDevID+3);
            if (iCardNum == io_pEndpointInfo->alsaInfo.cardNum) {
                io_pEndpointInfo->gsHALAPIName=strdup(pHalInfo->pAPIName);
                io_pEndpointInfo->gsDisplayName=strdup(pHalInfo->pDisplayName);
                return POLICY_SUCCESS;
            }            
        }
    }
    
    io_pEndpointInfo->gsHALAPIName=strdup(AHL_POLICY_UNDEFINED_HALNAME);
    io_pEndpointInfo->gsDisplayName=strdup(AHL_POLICY_UNDEFINED_DISPLAYNAME);
    
    return POLICY_FAIL;
}

static int GetHALList(void)
{
    json_object *j_response, *j_query = NULL;
    int err; 
    err = afb_service_call_sync("alsacore", "hallist", j_query, &j_response);
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
int Policy_OpenStream(json_object *pPolicyStreamJ)
{
    StreamInfoT PolicyStream;    
    EndpointInfoT EndpointInfo;
    PolicyStream.pEndpointInfo =&EndpointInfo;

    int err = PolicyCtxJSONToStream(pPolicyStreamJ, &PolicyStream);
    if(err == AHL_POLICY_UTIL_FAIL)
    {
        return AHL_POLICY_ACCEPT;     
    }

    // Example rule -> when system is in shutdown or low power mode, no audio stream can be opened (return AHL_POLICY_REJECT)
    // Should receive event from lower level layer
    if(g_PolicyCtx.systemState != SYSTEM_NORMAL)
    {
        return AHL_POLICY_REJECT;
    } 
  
    StreamConfigT StreamConfig;
    err = getStreamConfig(PolicyStream.pRoleName, &StreamConfig);
    if(err == POLICY_FAIL)
    {
        return AHL_POLICY_ACCEPT;     
    }

    if(PolicyStream.pEndpointInfo->deviceURIType != DEVICEURITYPE_NOT_ALSA) {
        err=PolicyGetVolume(PolicyStream.pEndpointInfo->endpointID, 
                            PolicyStream.pEndpointInfo->type,
                            PolicyStream.pEndpointInfo->gsHALAPIName, 
                            PolicyStream.pEndpointInfo->pRoleName, 
                            PolicyStream.pEndpointInfo->deviceURIType, 
                            &PolicyStream.pEndpointInfo->iVolume);
        if(err == POLICY_FAIL)
        {
            return AHL_POLICY_REJECT;
        }
    }

    err = PolicyAddEndPoint(&PolicyStream);
    if(err == POLICY_FAIL)
    {
        return AHL_POLICY_REJECT;
    }
    return AHL_POLICY_ACCEPT; 
}

int Policy_CloseStream(json_object *pPolicyStreamJ)
{
    //TODO remove Endpoint when there is no stream
    StreamInfoT PolicyStream;    
    EndpointInfoT EndpointInfo;
    PolicyStream.pEndpointInfo =&EndpointInfo;
    int err = PolicyCtxJSONToStream(pPolicyStreamJ, &PolicyStream);
    if(err == AHL_POLICY_UTIL_FAIL)
    {
        return AHL_POLICY_ACCEPT;     
    }

    return AHL_POLICY_ACCEPT; 
}

int  Policy_SetStreamState(json_object *pPolicyStreamJ)
{
    //TODO    
    // Optional: Mute endpoint before activation, unmute afterwards (after a delay?) to avoid noises
    StreamInfoT PolicyStream;    
    EndpointInfoT EndpointInfo;
    PolicyStream.pEndpointInfo =&EndpointInfo;


    StreamStateT streamState = 0;
    StreamInfoT * pPolicyStream = &PolicyStream;
    int err = PolicyCtxJSONToStream(pPolicyStreamJ, pPolicyStream);
    if(err == AHL_POLICY_UTIL_FAIL)
    {
        return AHL_POLICY_ACCEPT;     
    }

    json_object *streamStateJ=NULL;

    if(!json_object_object_get_ex(pPolicyStreamJ, "arg_stream_state", &streamStateJ))
    {
        return AHL_POLICY_ACCEPT;     
    }
    streamState = (StreamStateT)json_object_get_int(streamStateJ);

    //Change of state
    if(pPolicyStream->streamState != streamState)
    {
        //seach corresponding endpoint and gather information on it        
        EndPointPolicyInfoT *pCurrEndPointPolicy = PolicySearchEndPoint(pPolicyStream->pEndpointInfo->type , pPolicyStream->pEndpointInfo->gsDeviceName);
    
        switch(pPolicyStream->streamState)
        {
            case STREAM_STATE_IDLE:            
                switch(streamState)
                {
                    case STREAM_STATE_RUNNING:
                        err = PolicyIdleRunningTransition(pCurrEndPointPolicy, pPolicyStream);
                        if(err)
                        {
                            return AHL_POLICY_REJECT;  
                        }                        
                        PolicyPostStateEvent(pPolicyStream->streamID,STREAM_EVENT_START); 
                        break;
                    case STREAM_STATE_PAUSED:
                        err = PolicyIdleRunningTransition(pCurrEndPointPolicy, pPolicyStream);
                        if(err)
                        {
                            return AHL_POLICY_REJECT;  
                        }  
                        PolicyPostStateEvent(pPolicyStream->streamID,STREAM_EVENT_PAUSE); 
                        break;
                    default:
                        return AHL_POLICY_REJECT;  
                        break;
                }
                break;
            case STREAM_STATE_RUNNING:    
                switch(streamState)
                {
                    case STREAM_STATE_IDLE:
                        err = PolicyRunningIdleTransition(pCurrEndPointPolicy, pPolicyStream);
                        if(err)
                        {
                            return AHL_POLICY_REJECT;  
                        }                                            
                        PolicyPostStateEvent(pPolicyStream->streamID,STREAM_EVENT_STOP);                                       
                        break;
                    case STREAM_STATE_PAUSED:
                        PolicyPostStateEvent(pPolicyStream->streamID,STREAM_EVENT_PAUSE);                                       
                        break;
                    default:
                        return AHL_POLICY_REJECT;  
                        break;
                }
                break;
            case STREAM_STATE_PAUSED:    
                switch(streamState)
                {
                    case STREAM_STATE_IDLE:
                        err = PolicyRunningIdleTransition(pCurrEndPointPolicy, pPolicyStream);
                        if(err)
                        {
                            return AHL_POLICY_REJECT;  
                        }                        
                        PolicyPostStateEvent(pPolicyStream->streamID,STREAM_EVENT_STOP);    
                        break;
                    case STREAM_STATE_RUNNING:
                        PolicyPostStateEvent(pPolicyStream->streamID,STREAM_EVENT_RESUME); 
                        break;                 
                    default:
                        return AHL_POLICY_REJECT;  
                        break;
                }
                break;
            default:
                return AHL_POLICY_REJECT;  
                break;
        }      
    }
    return AHL_POLICY_ACCEPT; 
}

int  Policy_SetStreamMute(json_object *pPolicyStreamJ)
{
    StreamMuteT streamMute = 0;
    StreamInfoT PolicyStream;    
    EndpointInfoT EndpointInfo;
    PolicyStream.pEndpointInfo =&EndpointInfo;
    StreamInfoT * pPolicyStream = &PolicyStream;

    int err = PolicyCtxJSONToStream(pPolicyStreamJ, pPolicyStream);
    if(err == AHL_POLICY_UTIL_FAIL)
    {
        return AHL_POLICY_ACCEPT;     
    }

    json_object *streamMuteJ=NULL;

    if(!json_object_object_get_ex(pPolicyStreamJ, "mute_state", &streamMuteJ))
    {
        return AHL_POLICY_ACCEPT;     
    }
    streamMute = (StreamMuteT)json_object_get_int(streamMuteJ);

    if(streamMute != pPolicyStream->streamMute)
    {
        if(streamMute == STREAM_MUTED)
        {

            err= PolicySetVolume(pPolicyStream->pEndpointInfo->endpointID, 
                                pPolicyStream->pEndpointInfo->type,
                                pPolicyStream->pEndpointInfo->gsHALAPIName,
                                pPolicyStream->pRoleName,
                                pPolicyStream->pEndpointInfo->deviceURIType,
                                0,                                    
                                true);
            if(err)                
            {
                AFB_ERROR("StreamID:%i Set Volume return with errorcode%i",pPolicyStream->streamID, err);                        
                return AHL_POLICY_REJECT;  
            }   
            PolicyPostStateEvent(pPolicyStream->streamID,STREAM_EVENT_MUTED); 
        }
        else
        {
            err= PolicySetVolume(pPolicyStream->pEndpointInfo->endpointID,
                                pPolicyStream->pEndpointInfo->type,
                                pPolicyStream->pEndpointInfo->gsHALAPIName,
                                pPolicyStream->pRoleName,
                                pPolicyStream->pEndpointInfo->deviceURIType,                                    
                                pPolicyStream->pEndpointInfo->iVolume,
                                true);        
            if(err)                
            {
                AFB_ERROR("Endpoint:%i Set Volume return with errorcode%i",pPolicyStream->streamID, err);                        
                return AHL_POLICY_REJECT;  
            }   
            PolicyPostStateEvent(pPolicyStream->streamID,STREAM_EVENT_UNMUTED); 

        }

        pPolicyStream->streamMute = streamMute;
    }
    
    return AHL_POLICY_ACCEPT;
}

int Policy_SetVolume(json_object *pPolicyEndpointJ)
{
    char *volumeStr = NULL;
    EndpointInfoT EndpointInfo;
  
    int err = PolicyCtxJSONToEndpoint(pPolicyEndpointJ, &EndpointInfo);
    if(err == AHL_POLICY_UTIL_FAIL)
    {
        return AHL_POLICY_ACCEPT;     
    }

    json_object *volumeJ=NULL;

    if(!json_object_object_get_ex(pPolicyEndpointJ, "arg_volume", &volumeJ))
    {
        return AHL_POLICY_ACCEPT;     
    }
    volumeStr = (char*)json_object_get_string(volumeJ);

    // TODO: Parse volume string to support increment/absolute/percent notation (or delegate to action / policy layer to interpret)
    int vol = atoi(volumeStr);
    
    //Set the volume
    err= PolicySetVolume(EndpointInfo.endpointID, 
                         EndpointInfo.type,
                         EndpointInfo.gsHALAPIName,
                         EndpointInfo.pRoleName,
                         EndpointInfo.deviceURIType,                            
                         vol,
                         false);    
    if (err) 
    {
        AFB_ERROR("Set Volume return with errorcode%i", err);    
        return AHL_POLICY_REJECT;
    }

    return AHL_POLICY_ACCEPT; 
}

int Policy_SetProperty(json_object *pPolicyEndpointJ)
{

    char *propertyName = NULL;
    EndpointInfoT EndpointInfo;

    int err = PolicyCtxJSONToEndpoint(pPolicyEndpointJ, &EndpointInfo);
    if(err == AHL_POLICY_UTIL_FAIL)
    {
        return AHL_POLICY_ACCEPT;     
    }

    json_object *propertyNameJ=NULL;

    if(!json_object_object_get_ex(pPolicyEndpointJ, "arg_property_name", &propertyNameJ))
    {
        return AHL_POLICY_ACCEPT;     
    }
    propertyName = (char*)json_object_get_string(propertyNameJ);

    json_object *propValueJ;
    if(!json_object_object_get_ex(pPolicyEndpointJ, "arg_property_value", &propValueJ))
    {
        return AHL_POLICY_ACCEPT;     
    }

    gpointer *key_value=NULL;

    key_value = g_hash_table_lookup(EndpointInfo.pPropTable,propertyName);
    if(key_value == NULL)
    {
        AFB_ERROR("Can't find property %s, request will be rejected", propertyName);
        return AHL_POLICY_REJECT; 
    }

    //Get JsonObjectype
    json_type currentjType = json_object_get_type((json_object*)key_value);
    json_type newjType = json_object_get_type(propValueJ);

    //Apply policy on set property if needed here
    //Here we only validate that the type is the same
    if(currentjType != newjType)
    {
        AFB_ERROR("Property Value Type is wrong");
        return AHL_POLICY_REJECT; 
    }


    //Create a new Json Object    
    json_object *pEventDataJ = NULL;
    err = wrap_json_pack(&pEventDataJ,"{s:s,s:i,s:i,s:s,s:o,s:s}",
                        "event_name", AHL_ENDPOINT_PROPERTY_EVENT,
                        "endpoint_id", EndpointInfo.endpointID, 
                        "endpoint_type", EndpointInfo.type,
                        "property_name", propertyName,
                        "value",propValueJ, 
                        "audio_role", EndpointInfo.pRoleName);
    if(err)
    {
        AFB_ERROR("Unable to pack property event");        
        return AHL_POLICY_REJECT;
    }
    //Raise Event to update HLB
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

int Policy_Endpoint_Init(json_object *pPolicyEndpointJ)
{
    EndpointInfoT EndpointInfo;
  
    int err = PolicyCtxJSONToEndpoint(pPolicyEndpointJ, &EndpointInfo);
    if(err == AHL_POLICY_UTIL_FAIL)
    {
        return AHL_POLICY_REJECT;     
    }

    if (EndpointInfo.deviceURIType != DEVICEURITYPE_NOT_ALSA) {
        // Update Hal Name
        err = RetrieveAssociatedHALAPIName(&EndpointInfo);
        if (err) {
            AFB_ERROR("HAL not found for Device %s", EndpointInfo.gsDeviceName);
            return AHL_POLICY_REJECT;     
        }

        //Set Init Volume
        StreamConfigT StreamConfig;
        getStreamConfig(EndpointInfo.pRoleName, &StreamConfig);
        err = PolicySetVolume(EndpointInfo.endpointID, 
                            EndpointInfo.type,
                            EndpointInfo.gsHALAPIName, 
                            EndpointInfo.pRoleName, 
                            EndpointInfo.deviceURIType, 
                            StreamConfig.iVolumeInit,
                            false);
        if(err) {                                
            return AHL_POLICY_REJECT;     
        }
    }

    // Test example
    Add_Endpoint_Property_Int(&EndpointInfo,AHL_PROPERTY_EQ_LOW,3);
    Add_Endpoint_Property_Int(&EndpointInfo,AHL_PROPERTY_EQ_MID,0);
    Add_Endpoint_Property_Int(&EndpointInfo,AHL_PROPERTY_EQ_HIGH,6);
    Add_Endpoint_Property_Int(&EndpointInfo,AHL_PROPERTY_BALANCE,0);
    Add_Endpoint_Property_Int(&EndpointInfo,AHL_PROPERTY_FADE,30);
    Add_Endpoint_Property_String(&EndpointInfo,"preset_name","flat");


    gpointer *key_value = g_hash_table_lookup(EndpointInfo.pPropTable,AHL_PROPERTY_BALANCE);
    if(key_value == NULL)
    {
        AFB_ERROR("Can't find property %s, request will be rejected", AHL_PROPERTY_BALANCE);
        return AHL_POLICY_REJECT; 
    }

    //Create a new Json Object
    json_object *pNewPolicyEndpointJ = NULL;
    err = PolicyEndpointStructToJSON(&EndpointInfo, &pNewPolicyEndpointJ);
    if (err == AHL_POLICY_UTIL_FAIL)
    {
        return AHL_POLICY_REJECT;
    } 
    json_object *paramJ= json_object_new_string(AHL_ENDPOINT_INIT_EVENT);
    json_object_object_add(pNewPolicyEndpointJ, "event_name", paramJ);

    //Raise Event to update HLB
    audiohlapi_raise_event(pNewPolicyEndpointJ);

    return AHL_POLICY_ACCEPT; // No errors
}

int Policy_Init()
{
    // Initialize Ressources
    g_PolicyCtx.pSourceEndpoints =g_array_new(FALSE,TRUE,sizeof(EndPointPolicyInfoT));
    g_PolicyCtx.pSinkEndpoints = g_array_new(FALSE,TRUE,sizeof(EndPointPolicyInfoT));    
    g_PolicyCtx.pHALList = g_ptr_array_new_with_free_func(g_free);

    //Get HalList
    GetHALList();

    //Set System Normal for now, this should be set by an event
    //TODO: Receive event from low level
    g_PolicyCtx.systemState = SYSTEM_NORMAL;

    //register audio backend events
    //This is to simulate can bus, only used for demo
    json_object *queryurl, *responseJ, *eventsJ;
    
    eventsJ = json_object_new_array();
    json_object_array_add(eventsJ, json_object_new_string("audiod_system_event"));
    queryurl = json_object_new_object();
    json_object_object_add(queryurl, "events", eventsJ);
    int returnResult = afb_service_call_sync("audiod", "subscribe", queryurl, &responseJ);
    if (returnResult) {
        AFB_ERROR("Fail subscribing to Audio Backend System events");
        return AHL_POLICY_REJECT;
    }
    return AHL_POLICY_ACCEPT;
}
 
void Policy_Term()
{
    //Free Ressources
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
            //When speed change Modify volume on Endpoint where entertainment change
            PolicySpeedModify(speed);
        }
    }
}

#endif // AHL_DISCONNECT_POLICY