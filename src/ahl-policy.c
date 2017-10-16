/*
 * Copyright (C) 2017 "Audiokinetic Inc"
 * Author Francois Thibault <fthibault@audiokinetic.com>
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
#include "ahl-binding.h"
#include "wrap-json.h"

#define MAX_ACTIVE_STREAM_POLICY 30

// This file provides example of custom, business logic driven policy actions that can affect behavior of the high level
// TODO: Currently only isolated in separate source file. Objective is to make this to at least a shared lib plug-in (shared C context)
// Going a step further would be to implement this within a distinct policy binding (requires to switch to JSON interface)

extern AHLCtxT g_AHLCtx; // TODO: Cannot stay if moved to external module 

typedef struct StreamPolicyInfo {
    int             RolePriority;
    int             iVolume; 
    int             iVolumeSavedMute; 
    streamID_t      streamID;
    InterruptedBehaviorT interruptBehavior;                
} StreamPolicyInfoT;

 
typedef struct EndPointPolicyInfo {
    int             endpointKey;  
    endpointID_t    endpointID;
    EndpointTypeT   type;    
    GArray *        streamInfo; //List of playing or duck stream at a given endpoint
} EndPointPolicyInfoT;

typedef enum SystemState {
    SYSTEM_STARTUP = 0,     // Startup
    SYSTEM_SHUTDOWN,        // ShutDown
    SYSTEM_NORMAL,          // Normal
    SYSTEM_LOW_POWER,       // Low Power, save mode
    SYSTEM_MAXVALUE         // Enum count, keep at the end
} SystemStateT;


// Global Policy Local context
typedef struct PolicyLocalCtx {
    GArray *     pSourceEndpoints; // List of Source Endpoint with playing stream or interrupted stream
    GArray *     pSinkEndpoints;   // List of Sink Endpoint with playing stream or interrupted stream
    GArray *     pStreamOpenPerPolicy; //List of number of openstream per policy
    GArray *     pMaxStreamOpenPerPolicy; //List of number of openstream per policy
    GArray *     pVolDuckPerPolicy; //List of number of openstream per policy
    SystemStateT systemState;
} PolicyLocalCtxT;

PolicyLocalCtxT g_PolicyCtx;


//Helper Functions
static void Add_Endpoint_Property_Double( EndpointInfoT * io_pEndpointInfo, char * in_pPropertyName, double in_dPropertyValue)
{
    json_object * propValueJ = json_object_new_double(in_dPropertyValue);
    g_hash_table_insert(io_pEndpointInfo->pPropTable, in_pPropertyName, propValueJ);
}


static void Add_Endpoint_Property_Int( EndpointInfoT * io_pEndpointInfo, char * in_pPropertyName, int in_iPropertyValue)
{
    json_object * propValueJ = json_object_new_int(in_iPropertyValue);
    g_hash_table_insert(io_pEndpointInfo->pPropTable, in_pPropertyName, propValueJ);
}

static void Add_Endpoint_Property_String( EndpointInfoT * io_pEndpointInfo, char * in_pPropertyName, const char * in_pPropertyValue)
{
    json_object * propValueJ = json_object_new_string(in_pPropertyValue);
    g_hash_table_insert(io_pEndpointInfo->pPropTable, in_pPropertyName, propValueJ);
}

static int PolicySetVolume(EndpointInfoT * pEndpointInfo, int iVolume)
{
    
    // Using audio role available from endpoint to target the right HAL control (build string based on convention)
    GString * gsHALControlName;    
    switch(pEndpointInfo->deviceURIType)
    {
        case DEVICEURITYPE_ALSA_HW:
            gsHALControlName  = g_string_new("Master_Playback_Volume");    
            break;        
        case DEVICEURITYPE_ALSA_DMIX:
        case DEVICEURITYPE_ALSA_DSNOOP:
        case DEVICEURITYPE_ALSA_PLUG:
        case DEVICEURITYPE_ALSA_SOFTVOL:            
            gsHALControlName  = g_string_new(pEndpointInfo->gsAudioRole->str);
            g_string_append(gsHALControlName,"_Vol"); // Or _Vol for direct control (no ramping)    
            break;
        default:
            //Set volume to zero for display purpose only.
            //Not support yet
            AFB_WARNING("Endpoint %s is not support Device Type and can't set volume",pEndpointInfo->gsDeviceName->str);
            break;
    }

    // Set endpoint volume using HAL services (leveraging ramps etc.)
    json_object *j_response, *j_query = NULL;
 
    // Package query
    int err = wrap_json_pack(&j_query,"{s:s,s:i}","label",gsHALControlName->str, "val",iVolume);
    if (err) 
    {
        AFB_ERROR("Invalid query for HAL ctlset: %s",json_object_to_json_string(j_query));    
        return err;
    }

    //TODO implement Volume limitation based on policy

    // Set the volume using the HAL
    err = afb_service_call_sync(pEndpointInfo->gsHALAPIName->str, "ctlset", j_query, &j_response);
    if (err) 
    {
        AFB_ERROR("Could not ctlset on HAL: %s",pEndpointInfo->gsHALAPIName->str);    
        return err;
    }
    AFB_DEBUG("HAL ctlset response=%s", json_object_to_json_string(j_response));
 
     // Package event data
    json_object * eventDataJ = NULL;
    err = wrap_json_pack(&eventDataJ,"{s:i,s:i,s:i}","endpoint_id", pEndpointInfo->endpointID,"endpoint_type",pEndpointInfo->type,"value",iVolume);
    if (err) 
    {
        AFB_ERROR("Invalid event data for volume event, Invalid event data for volume event: %s",json_object_to_json_string(eventDataJ));    
        return err;
    }
    afb_event_push(g_AHLCtx.policyCtx.volumeEvent,eventDataJ);

    pEndpointInfo->iVolume = iVolume;

    return 0; 
}


static int PolicySetVolumeMute(EndpointInfoT * pEndpointInfo, int iVolume)
{
    
    // Using audio role available from endpoint to target the right HAL control (build string based on convention)
    GString * gsHALControlName;    
    switch(pEndpointInfo->deviceURIType)
    {
        case DEVICEURITYPE_ALSA_HW:
            gsHALControlName  = g_string_new("Master_Playback_Volume");    
            break;        
        case DEVICEURITYPE_ALSA_DMIX:
        case DEVICEURITYPE_ALSA_DSNOOP:
        case DEVICEURITYPE_ALSA_PLUG:
        case DEVICEURITYPE_ALSA_SOFTVOL:            
            gsHALControlName  = g_string_new(pEndpointInfo->gsAudioRole->str);
            g_string_append(gsHALControlName,"_Vol"); // Or _Vol for direct control (no ramping)    
            break;
        default:
            //Set volume to zero for display purpose only.
            //Not support yet
            AFB_WARNING("Endpoint %s is not support Device Type and can't set volume",pEndpointInfo->gsDeviceName->str);
            break;
    }

    // Set endpoint volume using HAL services (leveraging ramps etc.)
    json_object *j_response, *j_query = NULL;
 
    // Package query
    int err = wrap_json_pack(&j_query,"{s:s,s:i}","label",gsHALControlName->str, "val",iVolume);
    if (err) 
    {
        AFB_ERROR("Invalid query for HAL ctlset: %s",json_object_to_json_string(j_query));    
        return err;
    }

    //TODO implement Volume limitation based on policy

    // Set the volume using the HAL
    err = afb_service_call_sync(pEndpointInfo->gsHALAPIName->str, "ctlset", j_query, &j_response);
    if (err) 
    {
        AFB_ERROR("Could not ctlset on HAL: %s",pEndpointInfo->gsHALAPIName->str);    
        return err;
    }
    AFB_DEBUG("HAL ctlset response=%s", json_object_to_json_string(j_response));
 
     // Package event data
    json_object * eventDataJ = NULL;
    err = wrap_json_pack(&eventDataJ,"{s:i,s:i,s:i}","endpoint_id", pEndpointInfo->endpointID,"endpoint_type",pEndpointInfo->type,"value",iVolume);
    if (err) 
    {
        AFB_ERROR("Invalid event data for volume event, Invalid event data for volume event: %s",json_object_to_json_string(eventDataJ));    
        return err;
    }
    afb_event_push(g_AHLCtx.policyCtx.volumeEvent,eventDataJ);    

    return 0; 
}


static int PolicySetVolumeRamp(EndpointInfoT * pEndpointInfo, int iVolume)
{
    
    // Using audio role available from endpoint to target the right HAL control (build string based on convention)
    GString * gsHALControlName;        
    switch(pEndpointInfo->deviceURIType)
    {
        case DEVICEURITYPE_ALSA_HW:
            gsHALControlName  = g_string_new("Master_Ramp");    
            break;        
        case DEVICEURITYPE_ALSA_DMIX:
        case DEVICEURITYPE_ALSA_DSNOOP:
        case DEVICEURITYPE_ALSA_PLUG:
        case DEVICEURITYPE_ALSA_SOFTVOL:            
            gsHALControlName  = g_string_new(pEndpointInfo->gsAudioRole->str);
            g_string_append(gsHALControlName,"_Ramp"); // Or _Vol for direct control (no ramping)    
            break;
        default:
            //Set volume to zero for display purpose only.
            //Not support yet
            AFB_WARNING("Endpoint %s is not a support Device Type and can't set volume",pEndpointInfo->gsDeviceName->str);
            break;
    }

    // Set endpoint volume using HAL services (leveraging ramps etc.)
    json_object *j_response, *j_query = NULL;
 
    // Package query
    int err = wrap_json_pack(&j_query,"{s:s,s:i}","label",gsHALControlName->str, "val",iVolume);
    if (err) 
    {
        AFB_WARNING("Invalid query for HAL ctlset: %s",json_object_to_json_string(j_query));    
        return err;
    }

    //TODO implement Volume limitation based on policy

    // Set the volume using the HAL
    err = afb_service_call_sync(pEndpointInfo->gsHALAPIName->str, "ctlset", j_query, &j_response);
    if (err) 
    {
        AFB_WARNING("Could not ctlset on HAL: %s",pEndpointInfo->gsHALAPIName->str);    
        return err;
    }
    AFB_DEBUG("HAL ctlset response=%s", json_object_to_json_string(j_response));
 
     // Package event data
    json_object * eventDataJ = NULL;
    err = wrap_json_pack(&eventDataJ,"{s:i,s:i,s:i, s:s}","endpoint_id", pEndpointInfo->endpointID,"endpoint_type",pEndpointInfo->type,"value",iVolume, "audio_role",gsHALControlName->str);
    if (err) 
    {
        AFB_WARNING("Invalid event data for volume event, Invalid event data for volume event: %s",json_object_to_json_string(eventDataJ));    
        return err;
    }
    afb_event_push(g_AHLCtx.policyCtx.volumeEvent,eventDataJ);

    pEndpointInfo->iVolume = iVolume;

    return 0; 
}

static int PolicyGetVolume(EndpointInfoT * pEndpointInfo)
{
    

    GString * gsHALControlName;

    // Using audio role available from endpoint to target the right HAL control (build string based on convention)        
    switch(pEndpointInfo->deviceURIType)
    {
        case DEVICEURITYPE_ALSA_HW:
            gsHALControlName  = g_string_new("Master_Playback_Volume");    
            break;        
        case DEVICEURITYPE_ALSA_DMIX:
        case DEVICEURITYPE_ALSA_DSNOOP:
        case DEVICEURITYPE_ALSA_PLUG:
        case DEVICEURITYPE_ALSA_SOFTVOL:            
            gsHALControlName  = g_string_new(pEndpointInfo->gsAudioRole->str);
            g_string_append(gsHALControlName,"_Vol"); // Or _Vol for direct control (no ramping)    
            break;
        default:
            //Set volume to zero for display purpose only.
            //Not support yet
            pEndpointInfo->iVolume = 0;
            AFB_WARNING("Endpoint %s is a support Device Type and can't get volume",pEndpointInfo->gsDeviceName->str);
            break;
    }

    // Set endpoint volume using HAL services (leveraging ramps etc.)
    json_object *j_response, *j_query = NULL;
 
    // Package query
    int err = wrap_json_pack(&j_query,"{s:s}","label",gsHALControlName->str);
    if (err) 
    {
        AFB_WARNING("Invalid query for HAL ctlset: %s",json_object_to_json_string(j_query));    
        return err;
    }

    //TODO implement Volume limitation based on policy

    // Set the volume using the HAL
    err = afb_service_call_sync(pEndpointInfo->gsHALAPIName->str, "ctlget", j_query, &j_response);
    if (err) 
    {
        AFB_WARNING("Could not ctlset on HAL: %s",pEndpointInfo->gsHALAPIName->str);    
        return err;
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
            return -1;
        }
        
    }
    else
    {    
        err = wrap_json_unpack(jVal, "[i]", &val1);
        if (err) {
            AFB_ERROR("Volume retrieve failed Could not retrieve volume value -> %s", json_object_get_string(jVal));
            return -1;
        }
        
    }
        
    pEndpointInfo->iVolume = val1;

    return 0; 
}

static void PolicyPostStateEvent(struct afb_event streamStateEvent, StreamEventT eventState)
{
    
    json_object * eventDataJ = NULL;
    int err = wrap_json_pack(&eventDataJ,"{s:i}","stateEvent",eventState);
    if (err) 
    {
        AFB_ERROR("Invalid event data for stream state event: %s",json_object_to_json_string(eventDataJ));
    }
    else                    
    {
        afb_event_push(streamStateEvent,eventDataJ);
    }         
}

//This function is based on ALSA right now but it could be adapt to support other framework like pulseaudio or gstreamer
static int PolicyGenEndPointKey(EndpointInfoT *pEndPointInfo)
{
    return (pEndPointInfo->type << 24)|(pEndPointInfo->alsaInfo.cardNum << 16)|(pEndPointInfo->alsaInfo.deviceNum << 8)|(pEndPointInfo->alsaInfo.subDeviceNum);    
}

static EndPointPolicyInfoT *PolicySearchEndPoint(EndpointTypeT type, int key)
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

        if(pCurEndpoint->endpointKey == key)
        {
            return pCurEndpoint;
        }
    }

    return NULL;
}

static StreamPolicyInfoT *PolicySearchStream(EndPointPolicyInfoT * pCurEndpoint, int streamID)
{   

    for(int i=0; i<pCurEndpoint->streamInfo->len; i++)
    {
        StreamPolicyInfoT * pCurStream = &g_array_index(pCurEndpoint->streamInfo,StreamPolicyInfoT,i);    

        if(pCurStream->streamID == streamID)
        {
            return pCurStream;
        }
    }

    return NULL;
}


static StreamInfoT * PolicyGetActiveStream(streamID_t in_streamID)
{
    int iNumActiveStreams = g_AHLCtx.policyCtx.pActiveStreams->len;
    StreamInfoT * pStreamInfo = NULL;
    for ( int i = 0; i < iNumActiveStreams ; i++ ) {
        StreamInfoT * pCurStreamInfo = &g_array_index(g_AHLCtx.policyCtx.pActiveStreams,StreamInfoT,i);
        if (pCurStreamInfo->streamID == in_streamID){
            pStreamInfo = pCurStreamInfo;
            break;
        }
    }
    return pStreamInfo;
}


static int PolicyFindRoleIndex( const char * in_pAudioRole)
{
    int index = -1; // Not found
    for (int i = 0; i < g_AHLCtx.policyCtx.iNumberRoles; i++)
    {
        GString gs = g_array_index( g_AHLCtx.policyCtx.pAudioRoles, GString, i );
        if ( strcasecmp(gs.str,in_pAudioRole) == 0 )
        {
            index = i;
            break;
        }
    }
    return index;
}

static int PolicyRemoveStream(EndPointPolicyInfoT *pCurrEndPointPolicy, int RemoveIndex)
{
    //Validate 
    if(RemoveIndex >= pCurrEndPointPolicy->streamInfo->len)
    {
        return -1;

    }


    g_array_remove_index(pCurrEndPointPolicy->streamInfo,RemoveIndex);

    if(pCurrEndPointPolicy->streamInfo->len == 0)
    {

        //Free streem
        g_array_free(pCurrEndPointPolicy->streamInfo,TRUE);
        pCurrEndPointPolicy->streamInfo = NULL;
        
        GArray *pcurEndpointArray = NULL;

        if(pCurrEndPointPolicy->type==ENDPOINTTYPE_SINK)
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
            if(pCurEndpoint->endpointKey == pCurrEndPointPolicy->endpointKey)
            {
                g_array_remove_index(pcurEndpointArray, i);
                return 0;
            }            
        }
    }
    
    return 0;
}

static int PolicyRunningIdleTransition(EndPointPolicyInfoT *pCurrEndPointPolicy,StreamInfoT * pStreamInfo)
{
    if(pCurrEndPointPolicy == NULL)
    {
        //Remove endpoint
        AFB_ERROR("No Active Endpoint has been found");
        return -1; 
    }

    if(pCurrEndPointPolicy->streamInfo->len>0)
    {
        //Search for the matching stream
        int iNumStream = pCurrEndPointPolicy->streamInfo->len;
        for(int i=0; i<iNumStream; i++)
        {
            StreamPolicyInfoT currentPolicyStreamInfo = g_array_index(pCurrEndPointPolicy->streamInfo,StreamPolicyInfoT,i);
            if(currentPolicyStreamInfo.streamID == pStreamInfo->streamID)
            {

                //Unduck case
                if((i==(pCurrEndPointPolicy->streamInfo->len-1)) && (pCurrEndPointPolicy->streamInfo->len > 1)) 
                {
                    //remove the current stream
                    g_array_remove_index(pCurrEndPointPolicy->streamInfo, i);

                    //check the last element(Akways highest priority)
                    StreamPolicyInfoT HighPriorityStreamInfo = g_array_index(pCurrEndPointPolicy->streamInfo,StreamPolicyInfoT,pCurrEndPointPolicy->streamInfo->len-1);

                    //Get Stream Info
                    StreamInfoT * pInterruptStreamInfo = PolicyGetActiveStream(HighPriorityStreamInfo.streamID);
                    if (pInterruptStreamInfo == NULL) {
                        AFB_ERROR("Stream not found, Specified stream not currently active stream_id -> %d",HighPriorityStreamInfo.streamID);
                        return -1; 
                    }

                    int err;
                    switch(currentPolicyStreamInfo.interruptBehavior)
                    {
                        case AHL_INTERRUPTEDBEHAVIOR_CONTINUE:                                    
                            //unduck and set Volume back to original value
                            err= PolicySetVolumeRamp(pInterruptStreamInfo->pEndpointInfo, HighPriorityStreamInfo.iVolume);
                            if(err)                
                            {
                                AFB_ERROR("Endpoint:%s Set Volume return with errorcode%i", pInterruptStreamInfo->pEndpointInfo->gsDeviceName->str, err);                        
                                return -1; 
                            }                                            
                            break;
                        case AHL_INTERRUPTEDBEHAVIOR_PAUSE:
                            pInterruptStreamInfo->streamState = STREAM_STATE_RUNNING;
                            PolicyPostStateEvent(pInterruptStreamInfo->streamStateEvent,STREAM_EVENT_RESUME);                        
                            break;

                        case AHL_INTERRUPTEDBEHAVIOR_CANCEL:
                            AFB_ERROR("StreamID with Cancel InterruptedBehavior can't be unInterrupted");
                            return -1; 
                            break;
                        default:
                            AFB_ERROR("Unsupported Intterupt Behavior");
                            return -1; 
                            break;
                    } 

                }
                else                                                
                {
                    //remove the current stream
                    PolicyRemoveStream(pCurrEndPointPolicy, i);
                }                        
                return 0;
            }

        }  
    }

    AFB_ERROR("StreamID does not match any playing stream");
    return -1;
}

static int PolicyIdleRunningTransition(EndPointPolicyInfoT *pCurrEndPointPolicy, StreamInfoT * pStreamInfo, int AudioRoleIndex, int EndPointKey)
{
    int stream_priority;
    int ori_key_pos;
    
    bool bKeyFound=g_hash_table_lookup_extended(g_AHLCtx.policyCtx.pRolePriority,pStreamInfo->pEndpointInfo->gsAudioRole->str,&ori_key_pos,&stream_priority);                                        
    if(bKeyFound==false)
    {
        AFB_ERROR("Can't find stream priority, request will be rejected");
        return -1; 
    }
    
    //stream_priority = GPOINTER_TO_INT(value);


    InterruptedBehaviorT InterruptBehavior = g_array_index(g_AHLCtx.policyCtx.pInterruptBehavior,InterruptedBehaviorT,AudioRoleIndex);
    int err;
    if(pCurrEndPointPolicy == NULL) //No stream is playing on this endpoint
    {
        EndPointPolicyInfoT newEndPointPolicyInfo ;
        StreamPolicyInfoT newStreamPolicyInfo;

        //create EndPoint and add playing stream
        newEndPointPolicyInfo.endpointKey = EndPointKey;
        newEndPointPolicyInfo.endpointID = pStreamInfo->pEndpointInfo->endpointID;
        newEndPointPolicyInfo.type = pStreamInfo->pEndpointInfo->type;
        newEndPointPolicyInfo.streamInfo = g_array_new(FALSE,TRUE,sizeof(StreamPolicyInfoT));

        newStreamPolicyInfo.RolePriority = stream_priority;
        newStreamPolicyInfo.iVolume = pStreamInfo->pEndpointInfo->iVolume;
        newStreamPolicyInfo.streamID = pStreamInfo->streamID;                    
        newStreamPolicyInfo.interruptBehavior = InterruptBehavior;

        g_array_append_val(newEndPointPolicyInfo.streamInfo, newStreamPolicyInfo);                    
        g_array_append_val(g_PolicyCtx.pSinkEndpoints, newEndPointPolicyInfo);
        

        /*
        int *pVolume = &g_array_index(g_PolicyCtx.pVolInitPerPolicy,int,AudioRoleIndex);
        pStreamInfo->pEndpointInfo->iVolume = *pVolume;
        
        err= PolicySetVolumeRamp(pStreamInfo->pEndpointInfo, pStreamInfo->pEndpointInfo->iVolume);
        if(err)                
        {
            AFB_ERROR("Endpoint:%s Set Volume return with errorcode%i",pStreamInfo->pEndpointInfo->gsDeviceName->str, err);                        
            return err; 
        } */  
    }
    else
    {
        //Currently contains playing or duck stream
        if(pCurrEndPointPolicy->streamInfo->len > 0)
        {
            //check the last element 
            StreamPolicyInfoT HighPriorityStreamInfo = g_array_index(pCurrEndPointPolicy->streamInfo,StreamPolicyInfoT,pCurrEndPointPolicy->streamInfo->len-1);
            if((stream_priority) >= HighPriorityStreamInfo.RolePriority)
            {
                //Get Stream Info
                StreamInfoT * pInterruptStreamInfo = PolicyGetActiveStream(HighPriorityStreamInfo.streamID);
                if (pInterruptStreamInfo == NULL) {
                    AFB_ERROR("Stream not found Specified stream not currently active stream_id -> %d",HighPriorityStreamInfo.streamID);
                    return -1;
                }

                switch(InterruptBehavior)
                {
                    case AHL_INTERRUPTEDBEHAVIOR_CONTINUE:
                        //Save the current Volume and set the docking volume
                        HighPriorityStreamInfo.iVolume = pInterruptStreamInfo->pEndpointInfo->iVolume;
                        
                        int *pVolume = &g_array_index(g_PolicyCtx.pVolDuckPerPolicy,int,AudioRoleIndex);                        
                        err= PolicySetVolumeRamp(pInterruptStreamInfo->pEndpointInfo, *pVolume);
                        if(err)                
                        {
                            AFB_ERROR("Endpoint:%s Set Volume return with errorcode%i", pInterruptStreamInfo->pEndpointInfo->gsDeviceName->str, err);                        
                            return -1; 
                        }                                            
                        break;
                    case AHL_INTERRUPTEDBEHAVIOR_PAUSE:
                        pInterruptStreamInfo->streamState = STREAM_STATE_PAUSED;
                        PolicyPostStateEvent(pInterruptStreamInfo->streamStateEvent,STREAM_EVENT_PAUSE);                        
                        break;

                    case AHL_INTERRUPTEDBEHAVIOR_CANCEL:
                        pInterruptStreamInfo->streamState = STREAM_STATE_IDLE;
                        PolicyPostStateEvent(pInterruptStreamInfo->streamStateEvent,STREAM_EVENT_STOP);                        
                        g_array_remove_index(pCurrEndPointPolicy->streamInfo, pCurrEndPointPolicy->streamInfo->len-1);

                        break;
                    default:
                        AFB_ERROR("Unsupported Intterupt Behavior");
                        return AHL_POLICY_REJECT; 
                        break;

                } 

                //Add the playing stream at index 0
                StreamPolicyInfoT newStreamPolicyInfo;
                newStreamPolicyInfo.RolePriority = stream_priority;
                newStreamPolicyInfo.iVolume = pStreamInfo->pEndpointInfo->iVolume;
                newStreamPolicyInfo.streamID = pStreamInfo->streamID;                    
                newStreamPolicyInfo.interruptBehavior = InterruptBehavior;
                
                //Insert at the end, become highest priority streamID
                g_array_append_val(pCurrEndPointPolicy->streamInfo, newStreamPolicyInfo);  
                
                err= PolicySetVolumeRamp(pStreamInfo->pEndpointInfo, pStreamInfo->pEndpointInfo->iVolume);
                if(err)                
                {
                    AFB_ERROR("Endpoint:%s Set Volume return with errorcode%i", pInterruptStreamInfo->pEndpointInfo->gsDeviceName->str, err);                        
                    return err; 
                }                          

            }
            else
            {
                //Higher Priority Stream is playing
                AFB_NOTICE("Higher Priority Stream is playing");
                return -1; 
            }

        }
        else
        {
            //Remove endpoint
            AFB_ERROR("Active EndPoint is not attached to  any active stream");
            return -1; 

        }                    
    }

    return 0;
}

//Policy API
int Policy_OpenStream(StreamInfoT * pStreamInfo)
{
    // Example rule -> when system is in shutdown or low power mode, no audio stream can be opened (return AHL_POLICY_REJECT)
    // Should receive event from lower level layer
    if(g_PolicyCtx.systemState != SYSTEM_NORMAL)
    {
        return AHL_POLICY_REJECT;
    } 
        
   //Implement Policy open stream rules, limit to a certain number of stream open based on policy
   int index = PolicyFindRoleIndex(pStreamInfo->pEndpointInfo->gsAudioRole->str);
   int *pNumberOpenStream = &g_array_index(g_PolicyCtx.pStreamOpenPerPolicy,int,index);
   int MaxNumberOpenStream = g_array_index(g_PolicyCtx.pMaxStreamOpenPerPolicy,int,index);

    *pNumberOpenStream +=1;
    if((*pNumberOpenStream) > MaxNumberOpenStream )
    {
        return AHL_POLICY_REJECT;
    }

    //Get actual Volume
    int err=PolicyGetVolume(pStreamInfo->pEndpointInfo);
    if(err != 0)
    {
        AFB_WARNING("Can't get volume of Endpoint %s",pStreamInfo->pEndpointInfo->gsDeviceName->str);        
    }

    return AHL_POLICY_ACCEPT; 
}

int Policy_CloseStream(StreamInfoT * pStreamInfo)
{
    //Decrement the number of openstream
    int index = PolicyFindRoleIndex(pStreamInfo->pEndpointInfo->gsAudioRole->str);
    int *pNumberOpenStream = &g_array_index(g_PolicyCtx.pStreamOpenPerPolicy,int,index);

    *pNumberOpenStream -= 1;

    return AHL_POLICY_ACCEPT; 
}

int  Policy_SetStreamState(StreamInfoT * pStreamInfo, int AudioRoleIndex, StreamStateT streamState)
{
    //DONE
    // If higher priority audio role stream requires audio ducking (and un-ducking) of other streams (e.g. navigation ducks entertainment)
    // Could potentially provide a fairly generic system using interupt behavior information and audio role priority (implemented and can be customized here)
    // Specific exception can also be
    // Source exclusion. E.g. When a source (e.g tuner) with same audio role as already active stream (e.g. media player) with same endpoint target, 
    // the former source is stopped (i.e. raise streamstate stop event)
    // If source on communication role is active (e.g. handsfree call), activating entertainment sources is prohibited    
    // Startup or Shutdown audio stream mute entertainment (unmut when stream no longer active)

    //TODO    
    // Optional: Mute endpoint before activation, unmute afterwards (after a delay?) to avoid noises
    int err;

    //Change of state
    if(pStreamInfo->streamState != streamState)
    {
        //seach corresponding endpoint and gather information on it
        int key = PolicyGenEndPointKey(pStreamInfo->pEndpointInfo);    
        EndPointPolicyInfoT *pCurrEndPointPolicy = PolicySearchEndPoint(pStreamInfo->pEndpointInfo->type , key);
    
        switch(pStreamInfo->streamState)
        {
            case STREAM_STATE_IDLE:            
                switch(streamState)
                {
                    case STREAM_STATE_RUNNING:
                        err = PolicyIdleRunningTransition(pCurrEndPointPolicy, pStreamInfo, AudioRoleIndex, key);
                        if(err)
                        {
                            return AHL_POLICY_REJECT;  
                        }                        
                        pStreamInfo->streamState = STREAM_STATE_RUNNING;          
                        PolicyPostStateEvent(pStreamInfo->streamStateEvent,STREAM_EVENT_START); 
                        break;
                    case STREAM_STATE_PAUSED:
                        err = PolicyIdleRunningTransition(pCurrEndPointPolicy, pStreamInfo, AudioRoleIndex, key);
                        if(err)
                        {
                            return AHL_POLICY_REJECT;  
                        }  
                        pStreamInfo->streamState = STREAM_STATE_PAUSED;          
                        PolicyPostStateEvent(pStreamInfo->streamStateEvent,STREAM_EVENT_PAUSE); 
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
                        err = PolicyRunningIdleTransition(pCurrEndPointPolicy, pStreamInfo);
                        if(err)
                        {
                            return AHL_POLICY_REJECT;  
                        }                        
                        pStreamInfo->streamState = STREAM_STATE_IDLE;          
                        PolicyPostStateEvent(pStreamInfo->streamStateEvent,STREAM_EVENT_STOP);                                       
                        break;
                    case STREAM_STATE_PAUSED:
                        pStreamInfo->streamState = STREAM_STATE_PAUSED;          
                        PolicyPostStateEvent(pStreamInfo->streamStateEvent,STREAM_EVENT_PAUSE);                                       
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
                        err = PolicyRunningIdleTransition(pCurrEndPointPolicy, pStreamInfo);
                        if(err)
                        {
                            return AHL_POLICY_REJECT;  
                        }                        
                        pStreamInfo->streamState = STREAM_STATE_IDLE;          
                        PolicyPostStateEvent(pStreamInfo->streamStateEvent,STREAM_EVENT_STOP);    
                        break;
                    case STREAM_STATE_RUNNING:
                        pStreamInfo->streamState = STREAM_STATE_RUNNING;          
                        PolicyPostStateEvent(pStreamInfo->streamStateEvent,STREAM_EVENT_RESUME); 
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

int  Policy_SetStreamMute(StreamInfoT * pStreamInfo, StreamMuteT streamMute)
{
    int err;
  
    if(streamMute == STREAM_MUTED)
    {
        err= PolicySetVolumeMute(pStreamInfo->pEndpointInfo, 0);
        if(err)                
        {
            AFB_ERROR("Endpoint:%s Set Volume return with errorcode%i",pStreamInfo->pEndpointInfo->gsDeviceName->str, err);                        
            return AHL_POLICY_REJECT;  
        }   
        PolicyPostStateEvent(pStreamInfo->streamStateEvent,STREAM_EVENT_MUTED); 
    }
    else
    {
        err= PolicySetVolumeMute(pStreamInfo->pEndpointInfo, pStreamInfo->pEndpointInfo->iVolume);
        if(err)                
        {
            AFB_ERROR("Endpoint:%s Set Volume return with errorcode%i",pStreamInfo->pEndpointInfo->gsDeviceName->str, err);                        
            return AHL_POLICY_REJECT;  
        }   
        PolicyPostStateEvent(pStreamInfo->streamStateEvent,STREAM_EVENT_UNMUTED); 


    }

    pStreamInfo->streamMute = streamMute;

    return AHL_POLICY_ACCEPT;
}

int Policy_SetVolume(EndpointInfoT * f_pEndpointInfo, char *volumeStr)
{

    // TODO: Parse volume string to support increment/absolute/percent notation (or delegate to action / policy layer to interpret)
    int vol = atoi(volumeStr);
    
    //Set the volume
    int err = PolicySetVolumeRamp(f_pEndpointInfo, vol);
    if (err) 
    {
        AFB_ERROR("Set Volume return with errorcode%i", err);    
        return AHL_POLICY_REJECT;
    }

    return AHL_POLICY_ACCEPT; 
}

int Policy_SetProperty(EndpointInfoT * f_pEndpointInfo, char *propertyName, json_object *propValue)
{

    gpointer *key_value=NULL;
    key_value=g_hash_table_lookup(f_pEndpointInfo->pPropTable,propertyName);
    if(key_value==NULL)
    {
        AFB_ERROR("Can't find property %s, request will be rejected", propertyName);
        return AHL_POLICY_REJECT; 
    }

    // Object type detection for property value (string = state, numeric = property)
    json_type jType = json_object_get_type(propValue);
    switch (jType) {
        case json_type_double:
            Add_Endpoint_Property_Double(f_pEndpointInfo,propertyName,json_object_get_double(propValue));
        case json_type_int:
            Add_Endpoint_Property_Int(f_pEndpointInfo,propertyName,json_object_get_int(propValue));
        case json_type_string:
            Add_Endpoint_Property_String(f_pEndpointInfo,propertyName,json_object_get_string(propValue));
            break;
        default:
            AFB_ERROR("Invalid property argument Property value not a valid json object query=%s", json_object_get_string(propValue ));
            return AHL_POLICY_REJECT;
    }

    return AHL_POLICY_ACCEPT;
}

int Policy_PostEvent(char *eventName, char *audioRole, char *mediaName, void *audioContext)
{
    // TODO: Any event with media specified should trigger action on provided rendering services (e.g. Wwise binding, gstreamer file player wrapper, MPDC? simple ALSA player (aplay)?)

    // Example (when the policy is hooked to CAN events). Post audio playback events other than safety during reverse gear engaged declined

    // Example post HMI audio role playback events declined when higher priority streams are active

    return AHL_POLICY_ACCEPT;
}

int Policy_AudioDeviceChange()
{
    // Allow or disallow a new audio endpoint to be used by the system
    // TODO: Policy assigns audio role(s) for device (or default)
    // TODO: Raise events to engage device switching if active stream in audio role assigned to the new endpoint
    
    return AHL_POLICY_ACCEPT; 
}

int Policy_Endpoint_Property_Init(EndpointInfoT * io_EndpointInfo)
{
    // Populate list of supported properties for specified endpoint (use helper functions)
    // Setup initial values for all properties  GHashTabl

    // TODO: Switch on different known endpoints to populate different properties

    // Test example
    Add_Endpoint_Property_Int(io_EndpointInfo,AHL_PROPERTY_EQ_LOW,3);
    Add_Endpoint_Property_Int(io_EndpointInfo,AHL_PROPERTY_EQ_MID,0);
    Add_Endpoint_Property_Int(io_EndpointInfo,AHL_PROPERTY_EQ_HIGH,6);
    Add_Endpoint_Property_Int(io_EndpointInfo,AHL_PROPERTY_BALANCE,0);
    Add_Endpoint_Property_Int(io_EndpointInfo,AHL_PROPERTY_FADE,30);
    Add_Endpoint_Property_String(io_EndpointInfo,"preset_name","flat");
    
    return 0; // No errors
}

int Policy_Init()
{
    // Initialize Ressources
    g_PolicyCtx.pSourceEndpoints =g_array_new(FALSE,TRUE,sizeof(EndPointPolicyInfoT));
    g_PolicyCtx.pSinkEndpoints = g_array_new(FALSE,TRUE,sizeof(EndPointPolicyInfoT));
    g_PolicyCtx.pStreamOpenPerPolicy = g_array_sized_new(FALSE, TRUE, sizeof(int), g_AHLCtx.policyCtx.iNumberRoles);
    g_PolicyCtx.pMaxStreamOpenPerPolicy = g_array_sized_new(FALSE, TRUE, sizeof(int), g_AHLCtx.policyCtx.iNumberRoles);
    g_PolicyCtx.pVolDuckPerPolicy = g_array_sized_new(FALSE, TRUE, sizeof(int), g_AHLCtx.policyCtx.iNumberRoles);

    int initial_value=0;
    int max_value=0;
    int vol_init_value=0;
    int vol_duck_value=0;
    GArray * pRoleDeviceArray = NULL;
    //Init the number of open stream    
    for(int i=0; i<g_AHLCtx.policyCtx.iNumberRoles; i++)
    {
        g_array_append_val(g_PolicyCtx.pStreamOpenPerPolicy, initial_value);
        
        GString gs = g_array_index( g_AHLCtx.policyCtx.pAudioRoles, GString, i );
        max_value = MAX_ACTIVE_STREAM_POLICY;
        if ( strcasecmp(gs.str,AHL_ROLE_WARNING) == 0 )
        {
            max_value = 4;
            vol_init_value=80; 
            vol_duck_value = 0;
        }
        else if ( strcasecmp(gs.str,AHL_ROLE_GUIDANCE) == 0 )
        {
            max_value = 10;
            vol_init_value=70; 
            vol_duck_value = 40;
        }
        else if ( strcasecmp(gs.str,AHL_ROLE_NOTIFICATION) == 0 )
        {
            max_value = 4;
            vol_init_value=80;
            vol_duck_value = 0; 
        }
        else if ( strcasecmp(gs.str,AHL_ROLE_COMMUNICATION) == 0 )
        {
            max_value = 10;
            vol_init_value=70; 
            vol_duck_value = 30;
        }
        else if ( strcasecmp(gs.str,AHL_ROLE_ENTERTAINMENT) == 0 )
        {
            max_value = MAX_ACTIVE_STREAM_POLICY;
            vol_init_value=60; 
            vol_duck_value = 40;         
        }
        else if ( strcasecmp(gs.str,AHL_ROLE_SYSTEM) == 0 )
        {
            max_value = 2;
            vol_init_value=100; 
            vol_duck_value = 0;
        }
        else if ( strcasecmp(gs.str,AHL_ROLE_STARTUP) == 0 )
        {
            max_value = 1;
            vol_init_value=90; 
            vol_duck_value = 0;
        }
        else if ( strcasecmp(gs.str,AHL_ROLE_SHUTDOWN) == 0 )
        {
            max_value = 1;
            vol_init_value=90; 
            vol_duck_value = 0;
        }

        g_array_append_val(g_PolicyCtx.pMaxStreamOpenPerPolicy, max_value);
        g_array_append_val(g_PolicyCtx.pVolDuckPerPolicy, vol_duck_value);

        //Get actual volume value
        pRoleDeviceArray = g_ptr_array_index( g_AHLCtx.policyCtx.pSinkEndpoints, i );
        if (pRoleDeviceArray != NULL) 
        {
            for ( int j = 0 ; j < pRoleDeviceArray->len; j++)
            {

                //Init all Volume
                EndpointInfoT * pEndpointInfo = &g_array_index(pRoleDeviceArray,EndpointInfoT,j);
                int err= PolicySetVolumeRamp(pEndpointInfo, vol_init_value);
                if(err)                
                {
                    AFB_WARNING("Endpoint:%s Set Volume return with errorcode%i",pEndpointInfo->gsDeviceName->str, err);                        
                    //try to read volume instead
                    err=PolicyGetVolume(pEndpointInfo);
                    if(err != 0)
                    {
                        AFB_WARNING("Can't get volume of Endpoint %s",pEndpointInfo->gsDeviceName->str);
                    }    
                }  

                /*
                EndpointInfoT * pEndpointInfo = &g_array_index(pRoleDeviceArray,EndpointInfoT,j);                
                int err=PolicyGetVolume(pEndpointInfo);
                if(err != 0)
                {
                    AFB_WARNING("Can't get volume of Endpoint %s",pEndpointInfo->gsDeviceName->str);
                }
*/                            
            }
        }
    }
 

    //Set System Normal for now, this should be set by an event
    //TODO: Receive event from low level
    g_PolicyCtx.systemState = SYSTEM_NORMAL;

    //register audio backend events
    json_object *queryurl, *responseJ, *devidJ, *eventsJ;
    
    eventsJ = json_object_new_array();
    json_object_array_add(eventsJ, json_object_new_string("audiod_system_event"));
    queryurl = json_object_new_object();
    json_object_object_add(queryurl, "events", eventsJ);
    int returnResult = afb_service_call_sync("audiod", "subscribe", queryurl, &responseJ);
    if (returnResult) {
        AFB_ERROR("Fail subscribing to Audio Backend System events");
        return -1;
    }


    return 0; // No errors
}
 
void Policy_Term()
{

    //Free Ressources
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

    g_array_free(g_PolicyCtx.pStreamOpenPerPolicy,TRUE);
    g_PolicyCtx.pStreamOpenPerPolicy = NULL;
    g_array_free(g_PolicyCtx.pMaxStreamOpenPerPolicy,TRUE);
    g_PolicyCtx.pMaxStreamOpenPerPolicy = NULL;
    g_array_free(g_PolicyCtx.pVolDuckPerPolicy, TRUE);
    g_PolicyCtx.pVolDuckPerPolicy = NULL;
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


            //Get Stream Info
            StreamInfoT * pActiveStreamInfo = PolicyGetActiveStream(pCurStream->streamID);
            if (pActiveStreamInfo == NULL) {
                AFB_WARNING("Stream not found, Specified stream not currently active stream_id -> %d",pCurStream->streamID);
                return; 
            }

            if(strcasecmp(pActiveStreamInfo->pEndpointInfo->gsAudioRole->str,AHL_ROLE_ENTERTAINMENT)==0)
            {

                if(speed > 30 && speed < 100)
                {
                    int volume =speed;
                    PolicySetVolumeRamp(pActiveStreamInfo->pEndpointInfo,volume);
                }
                

            }
            
        }

    }
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