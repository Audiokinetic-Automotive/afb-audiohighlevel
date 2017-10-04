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

#include "ahl-binding.h"
#include "ahl-apidef.h" // Generated from JSON OpenAPI
#include "wrap-json.h"

// Global high-level binding context
AHLCtxT g_AHLCtx;

//afb_req_context_set ??
// usr = afb_req_context_get(req);
// afb_req_context_clear(req);

static void AudioFormatStructToJSON(json_object **audioFormatJ, AudioFormatT * pAudioFormat)
{
    wrap_json_pack(audioFormatJ, "{s:i,s:i,s:i}",
                    "sample_rate", pAudioFormat->sampleRate, 
                    "num_channels", pAudioFormat->numChannels, 
                    "sample_type", pAudioFormat->sampleType);
}

// Helper macros/func for packaging JSON objects from C structures
static void EndpointInfoStructToJSON(json_object **endpointInfoJ, EndpointInfoT * pEndpointInfo)
{
    json_object *formatInfoJ = NULL;
    wrap_json_pack(endpointInfoJ, "{s:i,s:i,s:s,s:i}",
                    "endpoint_id", pEndpointInfo->endpointID, 
                    "endpoint_type", pEndpointInfo->type, 
                    "device_name", pEndpointInfo->gsDeviceName->str, 
                    "device_uri_type", pEndpointInfo->deviceURIType);
    AudioFormatStructToJSON(&formatInfoJ,&pEndpointInfo->format);
    json_object_object_add(*endpointInfoJ,"format",formatInfoJ);
}
 
static void StreamInfoStructToJSON(json_object **streamInfoJ, StreamInfoT * pStreamInfo)
{
    json_object *endpointInfoJ = NULL;
    EndpointInfoStructToJSON(&endpointInfoJ,pStreamInfo->pEndpointInfo);
    wrap_json_pack(streamInfoJ, "{s:i,s:i,s:i,s:s}", 
        "stream_id", pStreamInfo->streamID,
        "state", pStreamInfo->streamState,
        "mute", pStreamInfo->streamMute,
        "device_uri",pStreamInfo->pEndpointInfo->gsDeviceURI->str);
    json_object_object_add(*streamInfoJ,"endpoint_info",endpointInfoJ);
}


static EndpointInfoT * GetEndpointInfo(endpointID_t in_endpointID, EndpointTypeT in_endpointType)
{
    EndpointInfoT * pEndpointInfo = NULL;
    for (int i = 0; i < g_AHLCtx.policyCtx.iNumberRoles; i++)
    {
        GArray * pRoleDeviceArray = NULL;
        if (in_endpointType == ENDPOINTTYPE_SOURCE){
            pRoleDeviceArray = g_ptr_array_index( g_AHLCtx.policyCtx.pSourceEndpoints, i );
        }
        else{
            pRoleDeviceArray = g_ptr_array_index( g_AHLCtx.policyCtx.pSinkEndpoints, i );
        }
        for (int j = 0; j < pRoleDeviceArray->len; j++) {
            EndpointInfoT * pCurEndpointInfo = &g_array_index(pRoleDeviceArray,EndpointInfoT,j);
            if (pCurEndpointInfo->endpointID == in_endpointID) {
                pEndpointInfo = pCurEndpointInfo;
                break;
            }
        }
    }
    return pEndpointInfo;
}

static StreamInfoT * GetActiveStream(streamID_t in_streamID)
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

static int CreateEvents()
{
    int err = 0;

    g_AHLCtx.policyCtx.propertyEvent = afb_daemon_make_event(AHL_ENDPOINT_PROPERTY_EVENT);
    err = !afb_event_is_valid(g_AHLCtx.policyCtx.propertyEvent);
    if (err) {
        AFB_ERROR("Could not create endpoint property change event");
        err++;
    }

    g_AHLCtx.policyCtx.volumeEvent = afb_daemon_make_event(AHL_ENDPOINT_VOLUME_EVENT);
    err = !afb_event_is_valid(g_AHLCtx.policyCtx.volumeEvent);
    if (err) {
        AFB_ERROR("Could not create endpoint volume change event");
        err++;
    }

    g_AHLCtx.policyCtx.postEvent = afb_daemon_make_event(AHL_POST_EVENT);
    err = !afb_event_is_valid(g_AHLCtx.policyCtx.postEvent);
    if (err) {
        AFB_ERROR("Could not create post event call event");
        err++;
    }

    return err;
}

static void AhlBindingTerm()
{
    // Policy termination
    Policy_Term();

    // Events
    for (int i = 0; i < g_AHLCtx.policyCtx.pEventList->len; i++)
    {
        // For each event within the role
        GArray * pRoleEventArray = g_ptr_array_index( g_AHLCtx.policyCtx.pEventList, i );
        for (int j = 0 ; j < pRoleEventArray->len; j++)
        {
            GString * gsEventName = &g_array_index(pRoleEventArray,GString,j);
            g_string_free(gsEventName,TRUE);
        }
        g_array_free(pRoleEventArray,TRUE);
        pRoleEventArray = NULL;
    }
    g_ptr_array_free(g_AHLCtx.policyCtx.pEventList,TRUE);
    g_AHLCtx.policyCtx.pEventList = NULL;

    // Endpoints
    TermEndpoints();

    // TODO: Need to free g_strings in HAL list
    g_array_free(g_AHLCtx.pHALList,TRUE);
    g_hash_table_remove_all(g_AHLCtx.policyCtx.pRolePriority);
    g_hash_table_destroy(g_AHLCtx.policyCtx.pRolePriority);
    // TODO: Need to free g_strings in audio roles list
    g_array_free(g_AHLCtx.policyCtx.pAudioRoles,TRUE); 
    g_array_free(g_AHLCtx.policyCtx.pInterruptBehavior,TRUE);
    g_array_free(g_AHLCtx.policyCtx.pActiveStreams,TRUE);

    AFB_INFO("Audio high-level Binding succesTermination");
}

static streamID_t CreateNewStreamID()
{
    streamID_t newID = g_AHLCtx.nextStreamID;
    g_AHLCtx.nextStreamID++;
    return newID;
}

static int FindRoleIndex( const char * in_pAudioRole)
{
    int index = -1; // Not found
    for (int i = 0; i < g_AHLCtx.policyCtx.iNumberRoles; i++)
    {
        GString gs = g_array_index( g_AHLCtx.policyCtx.pAudioRoles, GString, i );
        if ( strcasecmp(gs.str,in_pAudioRole) == 0 )
            index = i;
    }
    return index;
}


// Binding initialization
PUBLIC int AhlBindingInit()
{
    int errcount = 0;
    int err = 0;

    // This API uses services from low level audio
    err = afb_daemon_require_api_v2("alsacore",1) ;
    if( err != 0 )
    {
        AFB_ERROR("Audio high level API requires alsacore API to be available");
        return 1;
    }

    // Parse high-level binding JSON configuration file (will build device lists)
    errcount += ParseHLBConfig();

    atexit(AhlBindingTerm);

    // This API uses services from low level audio
    errcount = afb_daemon_require_api_v2("alsacore",1) ;
    if( errcount != 0 )
    {
        AFB_ERROR("Audio high level API requires alsacore API to be available");
        return 1;
    }

    errcount += CreateEvents();

    // Parse high-level binding JSON configuration file (will build device lists)
    errcount += ParseHLBConfig();

    // Policy initialization
    errcount += Policy_Init();

    // Initialize list of active streams
    g_AHLCtx.policyCtx.pActiveStreams = g_array_new(FALSE,TRUE,sizeof(StreamInfoT));

    // TODO: Register for device changes ALSA low level audio service or HAL

    // TODO: Use AGL persistence framework to retrieve and set inital state/volumes/properties

    AFB_DEBUG("Audio high-level Binding success errcount=%d", errcount);
    return errcount;
}

PUBLIC void AhlOnEvent(const char *evtname, json_object *eventJ)
{
    // TODO: Implement handling events from HAL...
    AFB_DEBUG("AHL received event %s", evtname);
}

// TODO: OnEventFunction when it actually subscribe to other binding events
// TODO: Dynamic device handling 
// if HAL device availability change -> update source / sink list
// Call policy to attempt to assign role based on information (e.g. device name)
// Policy should also determine priority (insert device in right spot in the list)

PUBLIC void audiohlapi_get_sources(struct afb_req req)
{
    json_object *sourcesJ = NULL;
    json_object *sourceJ = NULL;
    json_object *queryJ = NULL;
    char * audioRole = NULL;
   
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:s}", "audio_role", &audioRole);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }

    AFB_DEBUG("Filtering devices according to specified audio role=%s", audioRole);
    // List device only for current audio role and device type (pick the role device list)
    int iRoleIndex = FindRoleIndex(audioRole);
    if ( iRoleIndex < 0)
    {
        afb_req_fail_f(req, "Invalid arguments", "Requested audio role does not exist in current configuration -> %s", json_object_get_string(queryJ));
        return;
    }
    else
    {
        sourcesJ = json_object_new_array();
        GArray * pRoleSourceDeviceArray = g_ptr_array_index( g_AHLCtx.policyCtx.pSourceEndpoints, iRoleIndex );
        int iNumberDevices = pRoleSourceDeviceArray->len;
        for ( int j = 0 ; j < iNumberDevices; j++)
        {
            EndpointInfoT sourceInfo = g_array_index(pRoleSourceDeviceArray,EndpointInfoT,j);
            EndpointInfoStructToJSON(&sourceJ, &sourceInfo);
            json_object_array_add(sourcesJ, sourceJ);
        }
    } 

    afb_req_success(req, sourcesJ, "List of sources");
}

PUBLIC void audiohlapi_get_sinks(struct afb_req req)
{
    json_object *sinksJ = NULL;
    json_object *sinkJ = NULL;
    json_object *queryJ = NULL;
    char * audioRole = NULL;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:s}", "audio_role", &audioRole);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }

    AFB_DEBUG("Filtering devices according to specified audio role=%s", audioRole);
    // List device only for current audio role and device type (pick the role device list)
    int iRoleIndex = FindRoleIndex(audioRole);
    if ( iRoleIndex < 0)
    {
        afb_req_fail_f(req, "Invalid arguments", "Requested audio role does not exist in current configuration -> %s", json_object_get_string(queryJ));
        return;
    }
    else
    {
        sinksJ = json_object_new_array();
        GArray * pRoleSinkDeviceArray = g_ptr_array_index( g_AHLCtx.policyCtx.pSinkEndpoints, iRoleIndex );
        int iNumberDevices = pRoleSinkDeviceArray->len;
        for ( int j = 0 ; j < iNumberDevices; j++)
        {
            EndpointInfoT sinkInfo = g_array_index(pRoleSinkDeviceArray,EndpointInfoT,j);
            EndpointInfoStructToJSON(&sinkJ, &sinkInfo);
            json_object_array_add(sinksJ, sinkJ);
        }
    } 

    afb_req_success(req, sinksJ, "List of sinks");
}

PUBLIC void audiohlapi_stream_open(struct afb_req req)
{
    json_object *streamInfoJ = NULL;
    StreamInfoT streamInfo;
    json_object *queryJ = NULL;
    char * audioRole = NULL;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    endpointID_t endpointID = AHL_UNDEFINED;
    int policyAllowed = AHL_POLICY_REJECT;
    EndpointInfoT * pEndpointInfo = NULL;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:s,s:i,s?i}", "audio_role", &audioRole, "endpoint_type", &endpointType, "endpoint_id", &endpointID);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = audio_role:%s endpoint_type:%d endpoint_id:%d", audioRole,endpointType,endpointID);

    int iRoleIndex = FindRoleIndex(audioRole);
    if (iRoleIndex < 0) {
        afb_req_fail_f(req, "Invalid audio role", "Audio role was not found in configuration -> %s",audioRole);
        return;
    }

    GArray * pRoleDeviceArray = NULL;
    if (endpointType == ENDPOINTTYPE_SOURCE){
        pRoleDeviceArray = g_ptr_array_index( g_AHLCtx.policyCtx.pSourceEndpoints, iRoleIndex );
    }
    else{
        pRoleDeviceArray = g_ptr_array_index( g_AHLCtx.policyCtx.pSinkEndpoints, iRoleIndex );
    }
    if (pRoleDeviceArray->len == 0) {
        afb_req_fail_f(req, "No available devices", "No available devices for role:%s and device type:%d",audioRole,endpointType);
        return;
    }

    if (endpointID == AHL_UNDEFINED)
    {
        // Assign a device based on configuration priority (first in the list for requested role and endpoint type)
        pEndpointInfo = &g_array_index(pRoleDeviceArray,EndpointInfoT,0);
        streamInfo.endpointSelMode = AHL_ENDPOINTSELMODE_AUTO;
    }
    else{
        streamInfo.endpointSelMode = AHL_ENDPOINTSELMODE_MANUAL;
        // Find specified endpoint ID in list of devices
        int iNumberDevices = pRoleDeviceArray->len;
        for ( int j = 0 ; j < iNumberDevices; j++)
        {
            pEndpointInfo = &g_array_index(pRoleDeviceArray,EndpointInfoT,j);
            if (pEndpointInfo->endpointID == endpointID) {
                break;
            }
        }
        if (pEndpointInfo == NULL) {
            afb_req_fail_f(req, "Endpoint not available", "Specified endpoint not available for role:%s and device type:%d endpoint id %d",audioRole,endpointType,endpointID);
            return;
        }
    }

    // Call policy to verify whether creating a new audio stream is allowed in current context and possibly take other actions
    policyAllowed = Policy_OpenStream(audioRole, endpointType, pEndpointInfo->endpointID);
    if (policyAllowed == AHL_POLICY_REJECT)
    {
        afb_req_fail(req, "Audio policy violation", "Open stream not allowed in current context");
        return;
    }

    // Create stream
    streamInfo.streamID = CreateNewStreamID(); // create new ID
    streamInfo.streamState = STREAM_STATUS_READY;
    streamInfo.streamMute = STREAM_UNMUTED;
    streamInfo.pEndpointInfo = pEndpointInfo;

    char streamEventName[128];
    snprintf(streamEventName,128,"ahl_streamstate_%d",streamInfo.streamID);

    streamInfo.streamStateEvent = afb_daemon_make_event(streamEventName);
    err = !afb_event_is_valid(streamInfo.streamStateEvent);
    if (err) {
        afb_req_fail(req, "Stream event creation failure", "Could not create stream specific state change event");
        return;
    }

    err = afb_req_subscribe(req,streamInfo.streamStateEvent);
    if (err) {
        afb_req_fail(req, "Stream event subscription failure", "Could not subscribe to stream specific state change event");
        return;
    }

    // Push stream on active stream list
    g_array_append_val( g_AHLCtx.policyCtx.pActiveStreams, streamInfo );

    StreamInfoStructToJSON(&streamInfoJ,&streamInfo);

    afb_req_success(req, streamInfoJ, "Stream info structure");
}

PUBLIC void audiohlapi_stream_close(struct afb_req req)
{
    json_object *queryJ = NULL;
    streamID_t streamID = AHL_UNDEFINED;
    int policyAllowed = AHL_POLICY_REJECT;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i}", "stream_id", &streamID);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = stream_id:%d", streamID);

    // TODO: Validate that the application ID from which the stream close is coming is the one that opened it, otherwise fail and do nothing

    // Call policy to verify whether creating a new audio stream is allowed in current context and possibly take other actions
    policyAllowed = Policy_CloseStream(streamID);
    if (policyAllowed == AHL_POLICY_REJECT)
    {
        afb_req_fail(req, "Audio policy violation", "Close stream not allowed in current context");
        return;
    }

    // Remove from active stream list (if present)
    int iNumActiveStreams = g_AHLCtx.policyCtx.pActiveStreams->len;
    int iStreamFound = 0;
    for ( int i = 0; i < iNumActiveStreams ; i++ ) {
        StreamInfoT streamInfo = g_array_index(g_AHLCtx.policyCtx.pActiveStreams,StreamInfoT,i);
        if (streamInfo.streamID == streamID){
            // Unsubscribe client from stream events
            char streamEventName[128];
            snprintf(streamEventName,128,"ahl_streamstate_%d",streamInfo.streamID);
            int iValid = afb_event_is_valid(streamInfo.streamStateEvent);
            if (iValid) {
                err = afb_req_unsubscribe(req,streamInfo.streamStateEvent);
                if (err) {
                    afb_req_fail(req, "Stream event subscription failure", "Could not subscribe to stream specific state change event");
                    return;
                }
            }
            else{
                AFB_WARNING("Stream event no longer valid and therefore not unsubscribed");
                break;
            }    

            g_array_remove_index(g_AHLCtx.policyCtx.pActiveStreams,i);
            iStreamFound = 1;
            break;
        }
    }

    if (iStreamFound == 0) {
        afb_req_fail_f(req, "Stream not found", "Specified stream not currently active stream_id -> %d",streamID);
        return;
    }

    afb_req_success(req, NULL, "Stream close completed");
}

 PUBLIC void audiohlapi_set_stream_state(struct afb_req req)
 {
    json_object *queryJ = NULL;
    streamID_t streamID = AHL_UNDEFINED;
    StreamStateT streamState = STREAM_STATUS_MAXVALUE;
    int policyAllowed = AHL_POLICY_REJECT;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:i}", "stream_id", &streamID,"state",&streamState);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = stream_id:%d, state:%d", streamID,streamState);

    StreamInfoT * pStreamInfo = GetActiveStream(streamID);
    if (pStreamInfo == NULL) {
        afb_req_fail_f(req, "Stream not found", "Specified stream not currently active stream_id -> %d",streamID);
        return;
    }

    policyAllowed = Policy_SetStreamState(streamID,streamState);
    if (policyAllowed == AHL_POLICY_REJECT)
    {
        afb_req_fail(req, "Audio policy violation", "Change stream state not allowed in current context");
        return;
    }

    pStreamInfo->streamState = streamState;
    // Package event data
    json_object * eventDataJ = NULL;
    err = wrap_json_pack(&eventDataJ,"{s:i,s:i}","mute",pStreamInfo->streamMute,"state",streamState);
    if (err) {
        afb_req_fail_f(req, "Invalid event data for stream state event", "Invalid event data for stream state event: %s",json_object_to_json_string(eventDataJ));
        return;
    }
    afb_event_push(pStreamInfo->streamStateEvent,eventDataJ);

    afb_req_success(req, NULL, "Set stream state");
 }

 PUBLIC void audiohlapi_set_stream_mute(struct afb_req req)
 {
    json_object *queryJ = NULL;
    streamID_t streamID = AHL_UNDEFINED;
    StreamMuteT muteState = STREAM_MUTE_MAXVALUE;
    int policyAllowed = AHL_POLICY_REJECT;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:i}", "stream_id", &streamID,"mute",&muteState);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = stream_id:%d, mute:%d", streamID,muteState);

    StreamInfoT * pStreamInfo = GetActiveStream(streamID);
    if (pStreamInfo == NULL) {
        afb_req_fail_f(req, "Stream not found", "Specified stream not currently active stream_id -> %d",streamID);
        return;
    }

    policyAllowed = Policy_SetStreamMute(streamID,muteState);
    if (policyAllowed == AHL_POLICY_REJECT)
    {
        afb_req_fail(req, "Audio policy violation", "Mute stream not allowed in current context");
        return;
    }

    pStreamInfo->streamMute = muteState;

    // Package event data
    json_object * eventDataJ = NULL;
    err = wrap_json_pack(&eventDataJ,"{s:i,s:i}","mute",muteState,"state",pStreamInfo->streamState);
    if (err) {
        afb_req_fail_f(req, "Invalid event data for stream state event", "Invalid event data for stream state event: %s",json_object_to_json_string(eventDataJ));
        return;
    }
    afb_event_push(pStreamInfo->streamStateEvent,eventDataJ);

    afb_req_success(req, NULL, "Set stream mute completed");
 }

PUBLIC void audiohlapi_get_stream_info(struct afb_req req)
 {
    json_object *queryJ = NULL;
    streamID_t streamID = AHL_UNDEFINED;
    json_object * streamInfoJ = NULL;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i}", "stream_id", &streamID);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = stream_id:%d", streamID);

    StreamInfoT * pStreamInfo = GetActiveStream(streamID);
    if (pStreamInfo == NULL) {
        afb_req_fail_f(req, "Stream not found", "Specified stream not currently active stream_id -> %d",streamID);
        return;
    }

    StreamInfoStructToJSON(&streamInfoJ,pStreamInfo);

    afb_req_success(req, streamInfoJ, "Get stream info completed");
 }

PUBLIC void audiohlapi_set_volume(struct afb_req req)
{
    json_object *queryJ = NULL;
    endpointID_t endpointID = AHL_UNDEFINED;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    char * volumeStr = NULL;
    int policyAllowed = AHL_POLICY_REJECT;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:i,s:s}", "endpoint_type", &endpointType,"endpoint_id",&endpointID,"volume",&volumeStr);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%d endpoint_id:%d volume:%s", endpointType,endpointID,volumeStr);

    // TODO: Parse volume string to support increment/absolute/percent notation (or delegate to action / policy layer to interpret)
    int vol = atoi(volumeStr);

    // TODO: Policy needs way to set cached endpoint volume value (eg.)
    policyAllowed = Policy_SetVolume(endpointType, endpointID, volumeStr); // TODO: Potentially retrieve modified value by policy (e.g. volume limit)
    if (!policyAllowed)
    {
        afb_req_fail(req, "Audio policy violation", "Set volume not allowed in current context");
        return;
    }

    // TODO: Cache HAL control name during device enumeration for efficiency
    EndpointInfoT * pEndpointInfo = GetEndpointInfo(endpointID,endpointType);
    if (pEndpointInfo == NULL)
    {
        afb_req_fail_f(req, "Endpoint not found", "Endpoint information not found for id:%d type%d",endpointID,endpointType);
        return;
    }

    // Using audio role available from endpoint to target the right HAL control (build string based on convention)
    GString * gsHALControlName  = g_string_new(pEndpointInfo->gsAudioRole->str);
    g_string_append(gsHALControlName,"_Ramp"); // Or _Vol for direct control (no ramping)

    // Set endpoint volume using HAL services (leveraging ramps etc.)
    json_object *j_response, *j_query = NULL;
 
    // Package query
    err = wrap_json_pack(&j_query,"{s:s,s:i}","label",gsHALControlName->str, "val",vol);
    if (err) {
        afb_req_fail_f(req, "Invalid query for HAL ctlset", "Invalid query for HAL ctlset: %s",json_object_to_json_string(j_query));
        return;
    }

    // TODO: Move this to ref implmentation policy
    // Set the volume using the HAL
    err = afb_service_call_sync(pEndpointInfo->gsHALAPIName->str, "ctlset", j_query, &j_response);
    if (err) {
        afb_req_fail_f(req, "HAL ctlset failure", "Could not ctlset on HAL: %s",pEndpointInfo->gsHALAPIName->str);
        return;
    }
    AFB_DEBUG("HAL ctlset response=%s", json_object_to_json_string(j_response));

    // Package event data
    json_object * eventDataJ = NULL;
    err = wrap_json_pack(&eventDataJ,"{s:i,s:i,s:i}","endpoint_id",endpointID,"endpoint_type",endpointType,"value",vol);
    if (err) {
        afb_req_fail_f(req, "Invalid event data for volume event", "Invalid event data for volume event: %s",json_object_to_json_string(eventDataJ));
        return;
    }
    afb_event_push(g_AHLCtx.policyCtx.volumeEvent,eventDataJ);
 
    afb_req_success(req, NULL, "Set volume completed");
}

PUBLIC void audiohlapi_get_volume(struct afb_req req)
{
    json_object *queryJ = NULL;
    endpointID_t endpointID = AHL_UNDEFINED;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    json_object * volumeJ = NULL;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:i}", "endpoint_type", &endpointType,"endpoint_id",&endpointID);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%d endpoint_id:%d", endpointType,endpointID);

    // TODO: Cache HAL control name during device enumeration for efficiency
    EndpointInfoT * pEndpointInfo = GetEndpointInfo(endpointID,endpointType);
    if (pEndpointInfo == NULL)
    {
        afb_req_fail_f(req, "Endpoint not found", "Endpoint information not found for id:%d type%d",endpointID,endpointType);
        return;
    }

    // Using audio role available from endpoint to target the right HAL control (build string based on convention)
    GString * gsHALControlName = g_string_new(pEndpointInfo->gsAudioRole->str);
    g_string_append(gsHALControlName,"_Vol"); // Use current value, not ramp target

    // Set endpoint volume using HAL services (leveraging ramps etc.)
    json_object *j_response, *j_query = NULL;
 
    // TODO: Returned cached endpoint volume value (controlled by policy)
    // Package query
    err = wrap_json_pack(&j_query,"{s:s}","label",gsHALControlName->str);
    if (err) {
        afb_req_fail_f(req, "Invalid query for HAL ctlget", "Invalid query for HAL ctlget: %s",json_object_to_json_string(j_query));
        return;
    }

    // TODO: Return cached value or move to policy
    // Set the volume using the HAL
    err = afb_service_call_sync(pEndpointInfo->gsHALAPIName->str, "ctlget", j_query, &j_response);
    if (err) {
        afb_req_fail_f(req, "HAL ctlget failure", "Could not ctlget on HAL: %s",pEndpointInfo->gsHALAPIName->str);
        return;
    }
    AFB_INFO("HAL ctlget response=%s", json_object_to_json_string(j_response));

    // Parse response
    json_object * jRespObj = NULL;
    json_object_object_get_ex(j_response, "response", &jRespObj);
    json_object * jVal = NULL;
    json_object_object_get_ex(jRespObj, "val", &jVal);
    int val1 = 0, val2 = 0; // Why 2 values?      
    err = wrap_json_unpack(jVal, "[ii]", &val1, &val2);
    if (err) {
        afb_req_fail_f(req,"Volume retrieve failed", "Could not retrieve volume value -> %s", json_object_get_string(jVal));
        return;
    }

    volumeJ = json_object_new_double((double)val1);

    afb_req_success(req, volumeJ, "Retrieved volume value");
}

// Properties 
PUBLIC void audiohlapi_get_list_properties(struct afb_req req)
{
    json_object *queryJ = NULL;
    endpointID_t endpointID = AHL_UNDEFINED;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    json_object * endpointPropsJ = NULL;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:i}", "endpoint_type", &endpointType,"endpoint_id",&endpointID);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%d endpoint_id:%d", endpointType,endpointID);

    EndpointInfoT * pEndpointInfo = GetEndpointInfo(endpointID,endpointType);
    if (pEndpointInfo == NULL)
    {
        afb_req_fail_f(req, "Endpoint not found", "Endpoint information not found for id:%d type%d",endpointID,endpointType);
        return;
    }

    // Build and return list of properties for specific endpoint
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init (&iter, pEndpointInfo->pPropTable);
    endpointPropsJ = json_object_new_array();
    while (g_hash_table_iter_next (&iter, &key, &value))
    {
        json_object * propJ = json_object_new_string(key);
        json_object_array_add(endpointPropsJ, propJ);   
    }

    afb_req_success(req, endpointPropsJ, "Retrieved property list for endpoint");
}

PUBLIC void audiohlapi_set_property(struct afb_req req)
{
    json_object *queryJ = NULL;
    endpointID_t endpointID = AHL_UNDEFINED;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    char * propertyName = NULL;
    char * propValueStr = NULL;
    int policyAllowed = AHL_POLICY_REJECT;

    // TODO: object type detection (string = state, numeric = property)
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:i,s:s,s:s}", "endpoint_type", &endpointType,"endpoint_id",&endpointID,"property_name",&propertyName,"value",&propValueStr);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%d endpoint_id:%d property_name:%s value:%s", endpointType,endpointID,propertyName,propValueStr);
  
    // TODO: Parse property value string to support increment/absolute/percent notation

    // Call policy to allow custom policy actions in current context
    policyAllowed = Policy_SetProperty(endpointType, endpointID, propertyName, propValueStr); // TODO: Potentially retrieve modified value by policy (e.g. parameter limit) 
    if (!policyAllowed)
    {
        afb_req_fail(req, "Audio policy violation", "Set endpoint property not allowed in current context");
        return;
    }

    // TODO: Policy to dispatch on right service target
    // TODO: Policy tp cache value in property list

    afb_event_push(g_AHLCtx.policyCtx.propertyEvent,queryJ);

    afb_req_success(req, NULL, "Set property completed");
}

PUBLIC void audiohlapi_get_property(struct afb_req req)
{
    json_object *queryJ = NULL;
    endpointID_t endpointID = AHL_UNDEFINED;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    char * propertyName = NULL;
    json_object *propertyValJ;
    double value = 0.0;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:i,s:s}", "endpoint_type", &endpointType,"endpoint_id",&endpointID,"property_name",&propertyName);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%d endpoint_id:%d property_name:%s", endpointType,endpointID,propertyName);

    // TODO: Retriev189e cached property value
    // TODO Account for properties with string types

    value = 93.0; // TODO: Get actual property value 
    propertyValJ = json_object_new_double(value);

    afb_req_success(req, propertyValJ, "Retrieved property value");
}

// Audio related events

PUBLIC void audiohlapi_get_list_events(struct afb_req req)
{
    json_object *queryJ = NULL;
    char * audioRole = NULL;
    json_object * roleEventsJ = NULL;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:s}", "audio_role",&audioRole);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = audio_role:%s",audioRole);

    // Build and return list of events for specific audio role
    int iRoleIndex = FindRoleIndex(audioRole);
    if (iRoleIndex < 0) {
        afb_req_fail_f(req, "Invalid audio role", "Audio role was not found in configuration -> %s",audioRole);
        return;
    }

    GArray *  pRoleEventArray = g_ptr_array_index( g_AHLCtx.policyCtx.pEventList, iRoleIndex );
    roleEventsJ = json_object_new_array();
    int iNumberEvents = pRoleEventArray->len;
    for ( int i = 0 ; i < iNumberEvents; i++)
    {
        GString gsEventName = g_array_index(pRoleEventArray,GString,i);
        json_object * eventJ = json_object_new_string(gsEventName.str);
        json_object_array_add(roleEventsJ, eventJ);
    }
    
    afb_req_success(req, roleEventsJ, "Retrieved event list for audio role");
}

PUBLIC void audiohlapi_post_event(struct afb_req req)
{
    json_object *queryJ = NULL;
    char * eventName = NULL;  
    char * audioRole = NULL;
    char * mediaName = NULL;
    json_object *eventContext = NULL;
    int policyAllowed = AHL_POLICY_REJECT;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:s,s:s,s?s,s?o}", "event_name", &eventName,"audio_role",&audioRole,"media_name",&mediaName,"event_context",&eventContext);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = event_name:%s audio_role:%s", eventName,audioRole);

    // Verify if known event for audio role
    int iRoleIndex = FindRoleIndex(audioRole);
    if (iRoleIndex < 0) {
        afb_req_fail_f(req, "Invalid audio role", "Audio role was not found in configuration -> %s",audioRole);
        return;
    }

    GArray * pRoleEventArray = NULL;
    pRoleEventArray = g_ptr_array_index( g_AHLCtx.policyCtx.pEventList, iRoleIndex );
    if (pRoleEventArray->len == 0) {
        afb_req_fail_f(req, "No available events", "No available events for role:%s",audioRole);
        return;
    }
    // Check to find specific event
    int iEventFound = 0;
    for (int i = 0; i < pRoleEventArray->len; i++)
    {
        GString gs = g_array_index( pRoleEventArray, GString, i );
        if ( strcasecmp(gs.str,eventName) == 0 )
        {
            iEventFound = 1;
            break;
        }
    }
    if (!iEventFound) {
        afb_req_fail_f(req, "Event not found for audio role", "Event not found for roke:%s",audioRole);
        return;
    }

    // Call policy to allow custom policy actions in current context (e.g. cancel playback)
    policyAllowed = Policy_PostEvent(eventName, audioRole, mediaName, (void*)eventContext); 
    if (!policyAllowed)
    {
        afb_req_fail(req, "Audio policy violation", "Post sound event not allowed in current context");
        return;
    }

    afb_event_push(g_AHLCtx.policyCtx.postEvent,queryJ);

    afb_req_success(req, NULL, "Posted sound event");
 }


// Monitoring
PUBLIC void audiohlapi_subscribe(struct afb_req req)
{
    json_object *queryJ = NULL;
    json_object * eventArrayJ = NULL;

    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:o}", "events", &eventArrayJ);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    
    int iNumEvents = json_object_array_length(eventArrayJ);
    for (int i = 0; i < iNumEvents; i++)
    {
        char * pEventName = NULL;
        json_object * jEvent = json_object_array_get_idx(eventArrayJ,i);
        pEventName = (char *)json_object_get_string(jEvent);
        if(!strcasecmp(pEventName, AHL_ENDPOINT_PROPERTY_EVENT)) {
			afb_req_subscribe(req, g_AHLCtx.policyCtx.propertyEvent);
            AFB_DEBUG("Client subscribed to endpoint property events");
		}
        else if(!strcasecmp(pEventName, AHL_ENDPOINT_VOLUME_EVENT)) {
			afb_req_subscribe(req, g_AHLCtx.policyCtx.volumeEvent);
            AFB_DEBUG("Client subscribed to endpoint volume events");
		}
        else if(!strcasecmp(pEventName, AHL_POST_EVENT)) {
			afb_req_subscribe(req, g_AHLCtx.policyCtx.postEvent);
            AFB_DEBUG("Client subscribed to post event calls events");
		}
        else {
			afb_req_fail(req, "failed", "Invalid event");
			return;
		}
    }

    afb_req_success(req, NULL, "Subscribe to events finished");
}

PUBLIC void audiohlapi_unsubscribe(struct afb_req req)
{
    json_object *queryJ = NULL;
    json_object * eventArrayJ = NULL;

    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:o}", "events", &eventArrayJ);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    
    int iNumEvents = json_object_array_length(eventArrayJ);
    for (int i = 0; i < iNumEvents; i++)
    {
        char * pEventName = NULL;
        json_object * jEvent = json_object_array_get_idx(eventArrayJ,i);
        pEventName = (char *)json_object_get_string(jEvent);
        if(!strcasecmp(pEventName, AHL_ENDPOINT_PROPERTY_EVENT)) {
			afb_req_unsubscribe(req, g_AHLCtx.policyCtx.propertyEvent);
            AFB_DEBUG("Client unsubscribed to endpoint property events");
		}
        else if(!strcasecmp(pEventName, AHL_ENDPOINT_VOLUME_EVENT)) {
			afb_req_unsubscribe(req, g_AHLCtx.policyCtx.volumeEvent);
            AFB_DEBUG("Client unsubscribed to endpoint volume events");
		}
        else if(!strcasecmp(pEventName, AHL_POST_EVENT)) {
			afb_req_unsubscribe(req, g_AHLCtx.policyCtx.postEvent);
            AFB_DEBUG("Client unsubscribed to post event calls events");
		}
        else {
			afb_req_fail(req, "failed", "Invalid event");
			return;
		}
    }

    afb_req_success(req, NULL, "Unsubscribe to events finished");
}
