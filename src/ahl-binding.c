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
    wrap_json_pack(endpointInfoJ, "{s:i,s:i,s:s,s:s,s:i}",
                    "endpoint_id", pEndpointInfo->endpointID, 
                    "endpoint_type", pEndpointInfo->type, 
                    "device_name", pEndpointInfo->gsDeviceName->str, 
                    "display_name", pEndpointInfo->gsDisplayName->str, 
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
        {
            index = i;
            break;
        }
    }
    return index;
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

static AHLClientCtxT * AllocateClientContext()
{
    AHLClientCtxT * pClientCtx = malloc(sizeof(AHLClientCtxT));
    pClientCtx->pEndpointAccessList = g_array_new(FALSE, TRUE, sizeof(endpointID_t));
    pClientCtx->pStreamAccessList = g_array_new(FALSE, TRUE, sizeof(streamID_t));
    return pClientCtx;
}

static void TerminateClientContext(void * ptr)
{
    AHLClientCtxT * pClientCtx = (AHLClientCtxT *) ptr;
    g_array_free( pClientCtx->pEndpointAccessList, TRUE);
    g_array_free( pClientCtx->pStreamAccessList, TRUE);
    free(ptr);
}

static int CheckStreamAccessControl(AHLClientCtxT * pClientCtx, streamID_t streamID)
{
    int iAccessControl = AHL_ACCESS_CONTROL_DENIED;
    for (int i = 0; i < pClientCtx->pStreamAccessList->len ; i++) {
        streamID_t iID = g_array_index(pClientCtx->pStreamAccessList,streamID_t,i);
        if (iID == streamID) {
            iAccessControl = AHL_ACCESS_CONTROL_GRANTED;
        }    
    }
    return iAccessControl;
}

static int CheckEndpointAccessControl(AHLClientCtxT * pClientCtx, endpointID_t endpointID)
{
    int iAccessControl = AHL_ACCESS_CONTROL_DENIED;
    for (int i = 0; i < pClientCtx->pEndpointAccessList->len ; i++) {
        endpointID_t iID = g_array_index(pClientCtx->pEndpointAccessList,endpointID_t,i);
        if (iID == endpointID) {
            iAccessControl = AHL_ACCESS_CONTROL_GRANTED;
        }    
    }
    return iAccessControl;
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
    g_AHLCtx.pHALList = NULL;
    g_hash_table_remove_all(g_AHLCtx.policyCtx.pRolePriority);
    g_hash_table_destroy(g_AHLCtx.policyCtx.pRolePriority);
    g_AHLCtx.policyCtx.pRolePriority = NULL;
    // TODO: Need to free g_strings in audio roles list
    g_array_free(g_AHLCtx.policyCtx.pAudioRoles,TRUE); 
    g_AHLCtx.policyCtx.pAudioRoles = NULL;
    g_array_free(g_AHLCtx.policyCtx.pInterruptBehavior,TRUE);
    g_AHLCtx.policyCtx.pInterruptBehavior = NULL;
    g_array_free(g_AHLCtx.policyCtx.pActiveStreams,TRUE);
    g_AHLCtx.policyCtx.pActiveStreams = NULL;

    AFB_INFO("Audio high-level Binding succesTermination");
}

// Binding initialization
PUBLIC int AhlBindingInit()
{
    int errcount = 0;

    atexit(AhlBindingTerm);

    // This API uses services from low level audio. TODO: This dependency can be removed.
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
    g_AHLCtx.iNumActiveClients = 0;
    g_AHLCtx.policyCtx.pActiveStreams = g_array_new(FALSE,TRUE,sizeof(StreamInfoT));

    // TODO: Use AGL persistence framework to retrieve and set initial volumes/properties

    AFB_DEBUG("Audio high-level Binding success errcount=%d", errcount);
    return errcount;
}

PUBLIC void AhlOnEvent(const char *evtname, json_object *eventJ)
{
    AFB_DEBUG("AHL received event %s", evtname);
    
    //forward to policy to handle events
    Policy_OnEvent(evtname, eventJ);
}

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

    // Check if there is already an existing context for this client
    AHLClientCtxT * pClientCtx = afb_req_context_get(req); // Retrieve client-specific data structure
    if (pClientCtx == NULL)
    {
        g_AHLCtx.iNumActiveClients++;
        pClientCtx = AllocateClientContext();
        afb_req_context_set(req, pClientCtx, TerminateClientContext);
    }

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

    // Create stream
    streamInfo.streamID = CreateNewStreamID(); // create new ID
    streamInfo.streamState = STREAM_STATE_IDLE;
    streamInfo.streamMute = STREAM_UNMUTED;
    streamInfo.pEndpointInfo = pEndpointInfo;

    // Call policy to verify whether creating a new audio stream is allowed in current context and possibly take other actions
    policyAllowed = Policy_OpenStream(&streamInfo);    
    if (policyAllowed == AHL_POLICY_REJECT)
    {
        afb_req_fail(req, "Audio policy violation", "Open stream not allowed in current context");
        return;
    }    

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

    // Add to client context stream ID and endpoint ID access rights
    g_array_append_val(pClientCtx->pStreamAccessList, streamInfo.streamID);
    g_array_append_val(pClientCtx->pEndpointAccessList, streamInfo.pEndpointInfo->endpointID);

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

    // Check if there is already an existing context for this client
    AHLClientCtxT * pClientCtx = afb_req_context_get(req); // Retrieve client-specific data structure
    if (pClientCtx == NULL)
    {
        afb_req_fail(req, "No context associated with the request", "No context associated with the request");
        return;
    }

    // Verify that this client can control the stream
    int iStreamAccessControl = CheckStreamAccessControl( pClientCtx, streamID );
    if (iStreamAccessControl == AHL_ACCESS_CONTROL_DENIED)
    {
        afb_req_fail(req, "Access control denied", "Close stream not allowed in current client context");
        return;
    }

    // Call policy to verify whether creating a new audio stream is allowed in current context and possibly take other actions
    StreamInfoT * pStreamInfo = GetActiveStream(streamID);
    if (pStreamInfo == NULL) {
        afb_req_fail_f(req, "Stream not found", "Specified stream not currently active stream_id -> %d",streamID);
        return;
    }

    policyAllowed = Policy_CloseStream(pStreamInfo);
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
                    afb_req_fail(req, "Stream event subscription failure", "Could not unsubscribe to stream specific state change event");
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

    // Find index for cases where there are multiple streams per client
    // Remove from client context stream ID and endpoint ID access rights
    for (int i = 0; i < pClientCtx->pStreamAccessList->len ; i++) {
        streamID_t iID = g_array_index(pClientCtx->pStreamAccessList,streamID_t,i);
        if (iID == streamID) {
            g_array_remove_index(pClientCtx->pStreamAccessList, i);
            g_array_remove_index(pClientCtx->pEndpointAccessList, i);
        }    
    }

    if (pClientCtx->pStreamAccessList->len == 0 && pClientCtx->pEndpointAccessList == 0) {
        // If no more streams/endpoints owner, clear session
        afb_req_context_clear(req);
        g_AHLCtx.iNumActiveClients--;
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

    // Check if there is already an existing context for this client
    AHLClientCtxT * pClientCtx = afb_req_context_get(req); // Retrieve client-specific data structure
    if (pClientCtx == NULL)
    {
        afb_req_fail(req, "No context associated with the request", "No context associated with the request");
        return;
    }

    // Verify that this client can control the stream
    int iStreamAccessControl = CheckStreamAccessControl( pClientCtx, streamID );
    if (iStreamAccessControl == AHL_ACCESS_CONTROL_DENIED)
    {
        afb_req_fail(req, "Access control denied", "Set stream state not allowed in current client context");
        return;
    }
    int AudioRoleIndex = FindRoleIndex(pStreamInfo->pEndpointInfo->gsAudioRole->str);

    policyAllowed = Policy_SetStreamState(pStreamInfo, AudioRoleIndex, streamState);    
    if (policyAllowed == AHL_POLICY_REJECT)
    {
        afb_req_fail(req, "Audio policy violation", "Change stream state not allowed in current context");
        return; 
    }

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

    // Check if there is already an existing context for this client
    AHLClientCtxT * pClientCtx = afb_req_context_get(req); // Retrieve client-specific data structure
    if (pClientCtx == NULL)
    {
        afb_req_fail(req, "No context associated with the request", "No context associated with the request");
        return;
    }

    // Verify that this client can control the stream
    int iStreamAccessControl = CheckStreamAccessControl( pClientCtx, streamID );
    if (iStreamAccessControl == AHL_ACCESS_CONTROL_DENIED)
    {
        afb_req_fail(req, "Access control denied", "Set stream mute state not allowed in current client context");
        return;
    }

    policyAllowed = Policy_SetStreamMute(pStreamInfo,muteState);    
    if (policyAllowed == AHL_POLICY_REJECT)
    {
        afb_req_fail(req, "Audio policy violation", "Mute stream not allowed in current context");
        return;
    }

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

    EndpointInfoT * pEndpointInfo = GetEndpointInfo(endpointID,endpointType);
    if (pEndpointInfo == NULL)
    {
        afb_req_fail_f(req, "Endpoint not found", "Endpoint information not found for id:%d type%d",endpointID,endpointType);
        return;
    }

    // Check if there is already an existing context for this client
    AHLClientCtxT * pClientCtx = afb_req_context_get(req); // Retrieve client-specific data structure
    if (pClientCtx == NULL)
    {
        afb_req_fail(req, "No context associated with the request", "No context associated with the request");
        return;
    }

    // Verify that this client can control the stream
    int iEndpointAccessControl = CheckEndpointAccessControl( pClientCtx, endpointID );
    if (iEndpointAccessControl == AHL_ACCESS_CONTROL_DENIED)
    {
        afb_req_fail(req, "Access control denied", "Set volume not allowed in current client context");
        return;
    }

    policyAllowed = Policy_SetVolume(pEndpointInfo, volumeStr);
    if (!policyAllowed)
    {
        afb_req_fail(req, "Audio policy violation", "Set volume not allowed in current context");
        return;
    }
 
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

    EndpointInfoT * pEndpointInfo = GetEndpointInfo(endpointID,endpointType);
    if (pEndpointInfo == NULL)
    {
        afb_req_fail_f(req, "Endpoint not found", "Endpoint information not found for id:%d type%d",endpointID,endpointType);
        return;
    }

    volumeJ = json_object_new_double((double)pEndpointInfo->iVolume);

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
    json_object * propValueJ = NULL;
    int policyAllowed = AHL_POLICY_REJECT;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:i,s:s,s:o}", "endpoint_type", &endpointType,"endpoint_id",&endpointID,"property_name",&propertyName,"value",&propValueJ);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%d endpoint_id:%d property_name:%s", endpointType,endpointID,propertyName);


    EndpointInfoT * pEndpointInfo = GetEndpointInfo(endpointID,endpointType);
    if (pEndpointInfo == NULL)
    {
        afb_req_fail_f(req, "Endpoint not found", "Endpoint information not found for id:%d type%d",endpointID,endpointType);
        return;
    }

    // Check if there is already an existing context for this client
    AHLClientCtxT * pClientCtx = afb_req_context_get(req); // Retrieve client-specific data structure
    if (pClientCtx == NULL)
    {
        afb_req_fail(req, "No context associated with the request", "No context associated with the request");
        return;
    }

    // Verify that this client can control the stream
    int iEndpointAccessControl = CheckEndpointAccessControl( pClientCtx, endpointID );
    if (iEndpointAccessControl == AHL_ACCESS_CONTROL_DENIED)
    {
        afb_req_fail(req, "Access control denied", "Set property not allowed in current client context");
        return;
    }

    // Call policy to allow custom policy actions in current context
    policyAllowed = Policy_SetProperty(pEndpointInfo, propertyName, propValueJ);     
    if (!policyAllowed)
    {
        afb_req_fail(req, "Audio policy violation", "Set endpoint property not allowed in current context");
        return;
    }

    afb_event_push(g_AHLCtx.policyCtx.propertyEvent,queryJ);

    afb_req_success(req, NULL, "Set property completed");
}

PUBLIC void audiohlapi_get_property(struct afb_req req)
{
    json_object *queryJ = NULL;
    endpointID_t endpointID = AHL_UNDEFINED;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    char * propertyName = NULL;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:i,s:s}", "endpoint_type", &endpointType,"endpoint_id",&endpointID,"property_name",&propertyName);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%d endpoint_id:%d property_name:%s", endpointType,endpointID,propertyName);

    EndpointInfoT * pEndpointInfo = GetEndpointInfo(endpointID,endpointType);
    if (pEndpointInfo == NULL)
    {
        afb_req_fail_f(req, "Endpoint not found", "Endpoint information not found for id:%d type%d",endpointID,endpointType);
        return;
    }

    // Retrieve cached property value
    json_object * propertyValJ = (json_object *)g_hash_table_lookup(pEndpointInfo->pPropTable,propertyName);
    if (propertyValJ == NULL) {
        afb_req_fail_f(req, "Property not found", "Property information not found: %s",propertyName);
        return;
    }

    json_object_get(propertyValJ); // Increase ref count so that framework does not free our JSON object

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
