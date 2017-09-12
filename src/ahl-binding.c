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

// Workshop development topics:
// - Associating streamID with a client id of afb. Has to be done with sessions?
// - Declaring dependencies on other binding services (examples)
// - json_object_put to free JSON objects? potential leaks currently
// - no events on new HAL registered to alsacore?
// - unregistering event subscription examples?
// - discussion how to use property and event system to dispatch external parameterization (e.g. Wwise/Fiberdyne)
// - discussion where to best isolate policy (controller or HLB plugin)
// - Other HLB attributes to pass through (e.g. interrupted behavior) 
// - DBScale TLV warning
// - GLib (internal) dependency
// - HAL registration dependency (initialization order)
// - Binding startup arguments for config file path
// - Can we use the same HAL for different card numbers?
// - Example use of volume ramping in HAL?
// - Binding termination function
// - AGL persistence framework?
// - How to provide API services with config.xml (secrets and all)


// Helper macros/func for packaging JSON objects from C structures
#define EndpointInfoStructToJSON(__JSON_OBJECT__, __ENDPOINTINFOSTRUCT__) \
    wrap_json_pack(&__JSON_OBJECT__, "{s:i,s:i,s:s,s:i}",\
                    "endpoint_id", __ENDPOINTINFOSTRUCT__.endpointID, \
                    "endpoint_type", __ENDPOINTINFOSTRUCT__.type, \
                    "device_name", __ENDPOINTINFOSTRUCT__.deviceName, \
                    "device_uri_type", __ENDPOINTINFOSTRUCT__.deviceURIType);
 
static void StreamInfoStructToJSON(json_object **streamInfoJ, StreamInfoT streamInfo)
{
    json_object *endpointInfoJ;
    EndpointInfoStructToJSON(endpointInfoJ,streamInfo.endpointInfo);
    wrap_json_pack(streamInfoJ, "{s:i}", "stream_id", streamInfo.streamID);
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
            index = i;
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

    // Initialize list of active streams
    g_AHLCtx.policyCtx.pActiveStreams = g_array_new(FALSE,TRUE,sizeof(StreamInfoT));

    // TODO: Register for device changes ALSA low level audio service

    // TODO: Perform other binding initialization tasks (e.g. broadcast service ready event?)

    // TODO: Use AGL persistence framework to retrieve and set inital state/volumes/properties

    AFB_DEBUG("Audio high-level Binding success errcount=%d", errcount);
    return errcount;
}

// TODO: AhlBindingTerm()?
// // TODO: Use AGL persistence framework to retrieve and set inital state/volumes

// TODO: OnEventFunction
// if ALSA device availability change -> update source / sink list
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

    sourcesJ = json_object_new_array();

    AFB_DEBUG("Filtering devices according to specified audio role=%s", audioRole);
    // Check that the role requested exists in current configuration
    int roleIndex = FindRoleIndex(audioRole);
    if ( roleIndex == -1)
    {
        afb_req_fail_f(req, "Invalid arguments", "Requested audio role does not exist in current configuration -> %s", json_object_get_string(queryJ));
        return;
    }
    // List device only for current audio role and device type (pick the role device list)
    int iRoleIndex = FindRoleIndex(audioRole);
    if (iRoleIndex >= 0)
    {
        GArray * pRoleSourceDeviceArray = g_ptr_array_index( g_AHLCtx.policyCtx.pSourceEndpoints, iRoleIndex );
        int iNumberDevices = pRoleSourceDeviceArray->len;
        for ( int j = 0 ; j < iNumberDevices; j++)
        {
            EndpointInfoT sourceInfo = g_array_index(pRoleSourceDeviceArray,EndpointInfoT,j);
            EndpointInfoStructToJSON(sourceJ, sourceInfo);
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

    sinksJ = json_object_new_array();

    AFB_DEBUG("Filtering devices according to specified audio role=%s", audioRole);
    // Check that the role requested exists in current configuration
    int roleIndex = FindRoleIndex(audioRole);
    if ( roleIndex == -1)
    {
        afb_req_fail_f(req, "Invalid arguments", "Requested audio role does not exist in current configuration -> %s", json_object_get_string(queryJ));
        return;
    }
    // List device only for current audio role and device type (pick the role device list)
    int iRoleIndex = FindRoleIndex(audioRole);
    if (iRoleIndex >= 0)
    {
        GArray * pRoleSinkDeviceArray = g_ptr_array_index( g_AHLCtx.policyCtx.pSinkEndpoints, iRoleIndex );
        int iNumberDevices = pRoleSinkDeviceArray->len;
        for ( int j = 0 ; j < iNumberDevices; j++)
        {
            EndpointInfoT sinkInfo = g_array_index(pRoleSinkDeviceArray,EndpointInfoT,j);
            EndpointInfoStructToJSON(sinkJ, sinkInfo);
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
    EndpointTypeT endpointType;
    endpointID_t endpointID = UNDEFINED_ID;
    int policyAllowed = 0;
    EndpointInfoT endpointInfo;
    
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

    if (endpointID == UNDEFINED_ID)
    {
        // Assign a device based on configuration priority (first in the list for requested role and endpoint type)
        endpointInfo = g_array_index(pRoleDeviceArray,EndpointInfoT,0);
    }
    else{
        // Find specified endpoint ID in list of devices
        int iNumberDevices = pRoleDeviceArray->len;
        int iEndpointFound = 0;
        for ( int j = 0 ; j < iNumberDevices; j++)
        {
            endpointInfo = g_array_index(pRoleDeviceArray,EndpointInfoT,j);
            if (endpointInfo.endpointID == endpointID) {
                iEndpointFound = 1;
                break;
            }
        }
        if (iEndpointFound == 0) {
            afb_req_fail_f(req, "Endpoint not available", "Specified endpoint not available for role:%s and device type:%d endpoint id %d",audioRole,endpointType,endpointID);
            return;
        }
    }

    // Call policy to verify whether creating a new audio stream is allowed in current context and possibly take other actions
    policyAllowed = Policy_OpenStream(audioRole, endpointType, endpointID);
    if (policyAllowed == AUDIOHL_POLICY_REJECT)
    {
        afb_req_fail(req, "Audio policy violation", "Open stream not allowed in current context");
        return;
    }

    // Create stream
    streamInfo.streamID = CreateNewStreamID(); // create new ID
    streamInfo.endpointInfo = endpointInfo;

    // Push stream on active stream list
    g_array_append_val( g_AHLCtx.policyCtx.pActiveStreams, streamInfo );

    StreamInfoStructToJSON(&streamInfoJ,streamInfo);

    afb_req_success(req, streamInfoJ, "Stream info structure");
}

PUBLIC void audiohlapi_stream_close(struct afb_req req)
{
    json_object *queryJ = NULL;
    streamID_t streamID = UNDEFINED_ID;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i}", "stream_id", &streamID);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = stream_id:%d", streamID);

    // TODO: Validate that the application ID from which the stream close is coming is the one that opened it, otherwise fail and do nothing

    // Remove from active stream list (if present)
    int iNumActiveStreams = g_AHLCtx.policyCtx.pActiveStreams->len;
    int iStreamFound = 0;
    for ( int i = 0; i < iNumActiveStreams ; i++ ) {
        StreamInfoT streamInfo = g_array_index(g_AHLCtx.policyCtx.pActiveStreams,StreamInfoT,i);
        if (streamInfo.streamID == streamID){
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

// Endpoints
PUBLIC void audiohlapi_set_volume(struct afb_req req)
{
    json_object *queryJ = NULL;
    endpointID_t endpointID = UNDEFINED_ID;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    char * volumeStr = NULL;
    int rampTimeMS = 0;
    int policyAllowed = 0;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:i,s:s,s?i}", "endpoint_type", &endpointType,"endpoint_id",&endpointID,"volume",&volumeStr,"ramp_time_ms",&rampTimeMS);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%d endpoint_id:%d volume:%s ramp_time_ms: %d", endpointType,endpointID,volumeStr,rampTimeMS);
   
    // TODO: Parse volume string to support increment/absolute/percent notation
    int vol = atoi(volumeStr);

    policyAllowed = Policy_SetVolume(endpointType, endpointID, volumeStr, rampTimeMS); // TODO: Potentially retrieve modified value by policy (e.g. volume limit)
    if (!policyAllowed)
    {
        afb_req_fail(req, "Audio policy violation", "Set volume not allowed in current context");
        return;
    }

     // TODO: Cache during device enumeration for efficiency
    EndpointInfoT * pEndpointInfo = GetEndpointInfo(endpointID,endpointType);
    if (pEndpointInfo == NULL)
    {
        afb_req_fail_f(req, "Endpoint not found", "Endpoint information not found for id:%d type%d",endpointID,endpointType);
        return;
    }

    // Using audio role available from endpoint to target the right HAL control (build string based on convention)
    char halControlName[AUDIOHL_MAXHALCONTROLNAME_LENGTH];
    strncpy(halControlName,pEndpointInfo->audioRole,AUDIOHL_MAXHALCONTROLNAME_LENGTH);
    strcat(halControlName,"_Ramp"); // Or _Vol for direct control (no ramping)

    // Set endpoint volume using HAL services (leveraging ramps etc.)
    json_object *j_response, *j_query = NULL;
 
    // Package query
    err = wrap_json_pack(&j_query,"{s:s,s:i}","label",halControlName, "val",vol);
    if (err) {
        afb_req_fail_f(req, "Invalid query for HAL ctlset", "Invalid query for HAL ctlset: %s",json_object_to_json_string(j_query));
        return;
    }

    // Set the volume using the HAL
    err = afb_service_call_sync(pEndpointInfo->halAPIName, "ctlset", j_query, &j_response);
    if (err) {
        afb_req_fail_f(req, "HAL ctlset failure", "Could not ctlset on HAL: %s",pEndpointInfo->halAPIName);
        return;
    }
    AFB_DEBUG("HAL ctlset response=%s", json_object_to_json_string(j_response));
 
    afb_req_success(req, NULL, "Set volume completed");
}

PUBLIC void audiohlapi_get_volume(struct afb_req req)
{
    json_object *queryJ = NULL;
    endpointID_t endpointID = UNDEFINED_ID;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    json_object *volumeJ;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:i}", "endpoint_type", &endpointType,"endpoint_id",&endpointID);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%d endpoint_id:%d", endpointType,endpointID);

    // TODO: Cache during device enumeration for efficiency
    EndpointInfoT * pEndpointInfo = GetEndpointInfo(endpointID,endpointType);
    if (pEndpointInfo == NULL)
    {
        afb_req_fail_f(req, "Endpoint not found", "Endpoint information not found for id:%d type%d",endpointID,endpointType);
        return;
    }

    // Using audio role available from endpoint to target the right HAL control (build string based on convention)
    char halControlName[AUDIOHL_MAXHALCONTROLNAME_LENGTH];
    strncpy(halControlName,pEndpointInfo->audioRole,AUDIOHL_MAXHALCONTROLNAME_LENGTH);
    strcat(halControlName,"_Vol"); // Use current value, not ramp target

    // Set endpoint volume using HAL services (leveraging ramps etc.)
    json_object *j_response, *j_query = NULL;
 
    // Package query
    err = wrap_json_pack(&j_query,"{s:s}","label",halControlName);
    if (err) {
        afb_req_fail_f(req, "Invalid query for HAL ctlget", "Invalid query for HAL ctlget: %s",json_object_to_json_string(j_query));
        return;
    }

    // Set the volume using the HAL
    err = afb_service_call_sync(pEndpointInfo->halAPIName, "ctlget", j_query, &j_response);
    if (err) {
        afb_req_fail_f(req, "HAL ctlget failure", "Could not ctlget on HAL: %s",pEndpointInfo->halAPIName);
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

PUBLIC void audiohlapi_set_property(struct afb_req req)
{
    json_object *queryJ = NULL;
    endpointID_t endpointID = UNDEFINED_ID;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    char * propertyName = NULL;
    char * propValueStr = NULL;
    int rampTimeMS = 0;
    int policyAllowed = 0;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:i,s:s,s:s,s?i}", "endpoint_type", &endpointType,"endpoint_id",&endpointID,"property_name",&propertyName,"value",&propValueStr,"ramp_time_ms",&rampTimeMS);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%d endpoint_id:%d property_name:%s value:%s ramp_time_ms:%d", endpointType,endpointID,propertyName,propValueStr,rampTimeMS);
  
    // TODO: Parse property value string to support increment/absolute/percent notation

    // Call policy to allow custom policy actions in current context
    policyAllowed = Policy_SetProperty(endpointType, endpointID, propertyName, propValueStr, rampTimeMS); // TODO: Potentially retrieve modified value by policy (e.g. parameter limit) 
    if (!policyAllowed)
    {
        afb_req_fail(req, "Audio policy violation", "Set endpoint property not allowed in current context");
        return;
    }

    // TODO: Set endpoint property (dispatch on right service target) 
    // Property targets (Internal,Wwise,Fiberdyne) (e.g. Wwise.ParamX, Fiberdyne.ParamY, Internal.ParamZ)
    // Cache value in property list
    // TBD

    afb_req_success(req, NULL, "Set property completed");
}

PUBLIC void audiohlapi_get_property(struct afb_req req)
{
    json_object *queryJ = NULL;
    endpointID_t endpointID = UNDEFINED_ID;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    char * propertyName = NULL;
    json_object *propertyValJ;
    double value = 0.0;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s?i,s:s}", "endpoint_type", &endpointType,"endpoint_id",&endpointID,"property_name",&propertyName);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%d endpoint_id:%d property_name:%s", endpointType,endpointID,propertyName);

    // TODO: Retrieve cached property value

    value = 93.0; // TODO: Get actual property value 
    propertyValJ = json_object_new_double(value);

    afb_req_success(req, propertyValJ, "Retrieved property value");
}

PUBLIC void audiohlapi_set_state(struct afb_req req)
{
    json_object *queryJ = NULL;
    endpointID_t endpointID = UNDEFINED_ID;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    char * stateName = NULL;
    char * stateValue = NULL;
    int policyAllowed = 0;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:i,s:s,s:s}", "endpoint_type", &endpointType,"endpoint_id",&endpointID,"state_name",&stateName,"state_value",&stateValue);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%d endpoint_id:%d state_name:%s state_value:%s", endpointType,endpointID,stateName,stateValue);

    // Check that state provided is within list of known state for this config
    char * pDefaultStateValue = g_hash_table_lookup(g_AHLCtx.pDefaultStatesHT, stateName);
    if (pDefaultStateValue == NULL)
    {
        afb_req_fail_f(req, "Invalid arguments", "State provided is not known to configuration query=%s", stateName);
        return;
    }

    // Call policy to allow custom policy actions in current context
    policyAllowed = Policy_SetState(endpointType, endpointID, stateName, stateValue); // TODO: Potentially retrieve modified value by policy (e.g. state change) 
    if (!policyAllowed)
    {
        afb_req_fail(req, "Audio policy violation", "Set endpoint state not allowed in current context");
        return;
    }

    // Change the state of the endpoint as requested

    afb_req_success(req, NULL, "Set endpoint state completed");
}

PUBLIC void audiohlapi_get_state(struct afb_req req)
{
    json_object *queryJ = NULL;
    endpointID_t endpointID = UNDEFINED_ID;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    json_object *stateValJ;
    char * stateName = NULL;
    char * stateValue = NULL;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:i,s:s}", "endpoint_type", &endpointType,"endpoint_id",&endpointID,"state_name",&stateName);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%d endpoint_id:%d state_name:%s", endpointType,endpointID,stateName);

    stateValJ = json_object_new_string(stateValue);
    // return cached value

    afb_req_success(req, stateValJ, "Retrieved state value");
}

// Sound events
PUBLIC void audiohlapi_post_sound_event(struct afb_req req)
{
    json_object *queryJ = NULL;
    char * eventName = NULL;
    char * mediaName = NULL;
    char * audioRole = NULL;
    json_object *audioContext = NULL;
    int policyAllowed = 0;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:s,s:s,s?s,s?o}", "event_name", &eventName,"audio_role",&audioRole,"media_name",&mediaName,"audio_context",&audioContext);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = event_name:%s audio_role:%s media_name:%s", eventName,audioRole,mediaName);

    // Call policy to allow custom policy actions in current context (e.g. cancel playback)
    policyAllowed = Policy_PostSoundEvent(eventName, audioRole, mediaName, (void*)audioContext); // TODO: Potentially retrieve modified value by policy (e.g. change media) 
    if (!policyAllowed)
    {
        afb_req_fail(req, "Audio policy violation", "Post sound event not allowed in current context");
        return;
    }

    // TODO: Post sound event to rendering services (e.g. gstreamer file player wrapper or simple ALSA player)

    afb_req_success(req, NULL, "Posted sound event");
 }


// Monitoring
PUBLIC void audiohlapi_subscribe(struct afb_req req)
{
    // json_object *queryJ = NULL;
    
    // queryJ = afb_req_json(req);

    // TODO: Iterate through array length, parsing the string value to actual events
    // TODO: Subscribe to appropriate events from other services

    afb_req_success(req, NULL, "Subscribe to events finished");
}

PUBLIC void audiohlapi_unsubscribe(struct afb_req req)
{
    // json_object *queryJ = NULL;
    
    // queryJ = afb_req_json(req);

    // TODO: Iterate through array length, parsing the string value to actual events
    // TODO: Unsubscribe to appropriate events from other services

    afb_req_success(req, NULL, "Subscribe to events finished");
}
