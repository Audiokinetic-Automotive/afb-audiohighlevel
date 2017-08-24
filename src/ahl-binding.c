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
#include <time.h>

#include "ahl-binding.h"
#include "ahl-apidef.h" // Generated from JSON OpenAPI
#include "wrap-json.h"

// TODO: json_object_put to free JSON objects? potential leaks currently

// Helper macros/func for packaging JSON objects from C structures

#define EndpointInfoStructToJSON(__JSON_OBJECT__, __ENDPOINTINFOSTRUCT__) \
    wrap_json_pack(&__JSON_OBJECT__, "{s:i,s:i,s:s}", "endpoint_id", __ENDPOINTINFOSTRUCT__.endpoint_id, "type", __ENDPOINTINFOSTRUCT__.type, "name", __ENDPOINTINFOSTRUCT__.name);

#define RoutingInfoStructToJSON(__JSON_OBJECT__, __ROUTINGINFOSTRUCT__) \
    wrap_json_pack(&__JSON_OBJECT__, "{s:i,s:i,s:i}", "routing_id", __ROUTINGINFOSTRUCT__.routing_id, "source_id", __ROUTINGINFOSTRUCT__.source_id, "sink_id", __ROUTINGINFOSTRUCT__.sink_id);

static void StreamInfoStructToJSON(json_object **streamInfoJ, StreamInfoT streamInfo)
{
    json_object *endpointInfoJ;
    EndpointInfoStructToJSON(endpointInfoJ,streamInfo.endpoint_info);
    wrap_json_pack(streamInfoJ, "{s:i,s:s}", "stream_id", streamInfo.stream_id, "pcm_name", streamInfo.pcm_name);
    json_object_object_add(*streamInfoJ,"endpoint_info",endpointInfoJ);
}

// Binding initialization
PUBLIC int AhlBindingInit()
{

    int errcount = 0;

    // Initialize list of available sources/sinks using lower level services
    errcount += EnumerateSources();
    errcount += EnumerateSinks();

    // TODO: Register for device changes from lower level services

    // TODO: Parse high-level binding configuration file

    // TODO: Perform other binding initialization tasks

    AFB_DEBUG("Audio high-level Binding success errcount=%d", errcount);
    return errcount;
}

PUBLIC void audiohlapi_get_sources(struct afb_req req)
{
    json_object *sourcesJ = NULL;
    json_object *sourceJ = NULL;
    json_object *queryJ = NULL;
    AudioRoleT audioRole = AUDIOROLE_MAXVALUE;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s?i}", "audio_role", &audioRole);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }

    if (audioRole != AUDIOROLE_MAXVALUE)
    {
        AFB_DEBUG("Filtering according to specified audio role=%d", audioRole);
    }

    // Fake run-time data for test purposes
    EndpointInfoT sources[3];
    sources[0].endpoint_id = 0;
    sources[0].type = 0;
    sources[0].name = "Source0";
    sources[1].endpoint_id = 1;
    sources[1].type = 1;
    sources[1].name = "Source1";
    sources[2].endpoint_id = 2;
    sources[2].type = 2;
    sources[2].name = "Source2";

    sourcesJ = json_object_new_array();
    for ( unsigned int i = 0 ; i < 3; i++)
    {
        EndpointInfoStructToJSON(sourceJ, sources[i]);
        json_object_array_add(sourcesJ, sourceJ);
    }

    afb_req_success(req, sourcesJ, "List of sources");
}

PUBLIC void audiohlapi_get_sinks(struct afb_req req)
{
    json_object *sinksJ = NULL;
    json_object *sinkJ = NULL;
    json_object *queryJ = NULL;
    AudioRoleT audioRole = AUDIOROLE_MAXVALUE;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s?i}", "audio_role", &audioRole);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }

    if (audioRole != AUDIOROLE_MAXVALUE)
    {
        AFB_DEBUG("Filtering according to specified audio role=%d", audioRole);
    }

    // Fake run-time data for test purposes
    EndpointInfoT sinks[3];
    sinks[0].endpoint_id = 0;
    sinks[0].type = 0;
    sinks[0].name = "Sink0";
    sinks[1].endpoint_id = 1;
    sinks[1].type = 1;
    sinks[1].name = "Sink1";
    sinks[2].endpoint_id = 2;
    sinks[2].type = 2;
    sinks[2].name = "Sink2";

    sinksJ = json_object_new_array();
    for ( unsigned int i = 0 ; i < 3; i++)
    {
        EndpointInfoStructToJSON(sinkJ, sinks[i]);
        json_object_array_add(sinksJ, sinkJ);
    }

    afb_req_success(req, sinksJ, "List of sinks");
}

PUBLIC void audiohlapi_stream_open(struct afb_req req)
{
    json_object *streamInfoJ = NULL;
    StreamInfoT streamInfo;
    json_object *queryJ = NULL;
    AudioRoleT audioRole;
    EndpointTypeT endpointType;
    endpointID_t endpointID = UNDEFINED_ID;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:i,s?i}", "audio_role", &audioRole, "endpoint_type", &endpointType, "endpoint_id", &endpointID);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = audio_role:%d endpointType:%d endpointID:%d", audioRole,endpointType,endpointID);

    if (endpointID == UNDEFINED_ID)
    {
        // TODO: Go through configuration and available devices to find best device for specified role
        endpointID = 2;
    }

    // Fake run-time data for test purposes
    streamInfo.stream_id = 12;
    streamInfo.pcm_name = "plug:Entertainment";
    streamInfo.endpoint_info.endpoint_id = endpointID;
    streamInfo.endpoint_info.type = endpointType;
    streamInfo.endpoint_info.name = "MainSpeakers"; 

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

    afb_req_success(req, NULL, "Stream close completed");
}

// Routings
PUBLIC void audiohlapi_get_available_routings(struct afb_req req)
{
    json_object *routingsJ;
    json_object *routingJ;
    json_object *queryJ = NULL;
    AudioRoleT audioRole = AUDIOROLE_MAXVALUE;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s?i}", "audio_role", &audioRole);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }

    if (audioRole != AUDIOROLE_MAXVALUE)
    {
        AFB_DEBUG("Filtering according to specified audio role=%d", audioRole);
    }

    // Fake run-time data for test purposes
    RoutingInfoT routings[3];
    routings[0].source_id = 0;
    routings[0].sink_id = 0;
    routings[0].routing_id = 0;
    routings[1].source_id = 1;
    routings[1].sink_id = 1;
    routings[1].routing_id = 1;
    routings[2].source_id = 2;
    routings[2].sink_id = 2;
    routings[2].routing_id = 2;

    routingsJ = json_object_new_array();
    for (unsigned int i = 0; i < 3; i++)
    {
        RoutingInfoStructToJSON(routingJ, routings[i]);
        json_object_array_add(routingsJ, routingJ);
    }

    afb_req_success(req, routingsJ, "List of available routings");
}

PUBLIC void audiohlapi_add_routing(struct afb_req req)
{
    json_object *queryJ = NULL;
    AudioRoleT audioRole = AUDIOROLE_MAXVALUE;
    routingID_t routingID = UNDEFINED_ID;
    json_object *routingJ = NULL;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s?i}", "audio_role", &audioRole,"routing_id",routingID);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = audio_role:%d routing_id:%d", audioRole,routingID);

    // Fake run-time data for test purposes
    RoutingInfoT routingInfo;
    routingInfo.routing_id = routingID;
    routingInfo.source_id = 3;
    routingInfo.sink_id = 4;

    RoutingInfoStructToJSON(routingJ,routingInfo);  

    afb_req_success(req,routingJ, "Selected routing information");
}

PUBLIC void audiohlapi_remove_routing(struct afb_req req)
{
    json_object *queryJ = NULL;
    routingID_t routingID = UNDEFINED_ID;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i}", "routing_id", &routingID);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = routing_id:%d", routingID);

    // TODO: Validate that the application ID from which the stream close is coming is the one that opened it, otherwise fail and do nothing

    afb_req_success(req, NULL, "Remove routing completed");
}

// Endpoints
PUBLIC void audiohlapi_set_endpoint_volume(struct afb_req req)
{
    json_object *queryJ = NULL;
    endpointID_t endpointID = UNDEFINED_ID;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    char * volumeStr = NULL;
    int rampTimeMS = 0;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:i,s:s,s?i}", "endpoint_type", &endpointType,"endpoint_id",&endpointID,"volume",&volumeStr,"ramp_time_ms",&rampTimeMS);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%d endpoint_id:%d volume:%s ramp_time_ms: %d", endpointType,endpointID,volumeStr,rampTimeMS);
  
    // TODO: Parse volume string to support increment/absolute/percent notation

    afb_req_success(req, NULL, "Set volume completed");
}

PUBLIC void audiohlapi_get_endpoint_volume(struct afb_req req)
{
    json_object *queryJ = NULL;
    endpointID_t endpointID = UNDEFINED_ID;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    json_object *volumeJ;
    double volume = 0.0;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:i}", "endpoint_type", &endpointType,"endpoint_id",&endpointID);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%d endpoint_id:%d", endpointType,endpointID);

    volume = 87.0; // TODO: Get actual volume value
    volumeJ = json_object_new_double(volume);

    afb_req_success(req, volumeJ, "Retrieved volume value");
}

PUBLIC void audiohlapi_set_endpoint_property(struct afb_req req)
{
    json_object *queryJ = NULL;
    endpointID_t endpointID = UNDEFINED_ID;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    char * propertyName = NULL;
    char * propValueStr = NULL;
    int rampTimeMS = 0;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:i,s:s,s:s,s?i}", "endpoint_type", &endpointType,"endpoint_id",&endpointID,"property_name",&propertyName,"value",&propValueStr,"ramp_time_ms",&rampTimeMS);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%d endpoint_id:%d property_name:%s value:%s ramp_time_ms:%d", endpointType,endpointID,propertyName,propValueStr,rampTimeMS);
  
    // TODO: Parse property value string to support increment/absolute/percent notation

    afb_req_success(req, NULL, "Set property completed");
}

PUBLIC void audiohlapi_get_endpoint_property(struct afb_req req)
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

    value = 93.0; // TODO: Get actual property value
    propertyValJ = json_object_new_double(value);

    afb_req_success(req, propertyValJ, "Retrieved property value");
}

PUBLIC void audiohlapi_set_endpoint_state(struct afb_req req)
{
    json_object *queryJ = NULL;
    endpointID_t endpointID = UNDEFINED_ID;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    char * stateName = NULL;
    char * stateValue = NULL;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:i,s:s,s:s}", "endpoint_type", &endpointType,"endpoint_id",&endpointID,"state_name",&stateName,"state_value",&stateValue);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%d endpoint_id:%d state_name:%s state_value:%s", endpointType,endpointID,stateName,stateValue);

    afb_req_success(req, NULL, "Set endpoint state completed");
}

PUBLIC void audiohlapi_get_endpoint_state(struct afb_req req)
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

    afb_req_success(req, stateValJ, "Retrieved state value");
}

// Sound events
PUBLIC void audiohlapi_post_sound_event(struct afb_req req)
{
    json_object *queryJ = NULL;
    char * eventName = NULL;
    char * mediaName = NULL;
    AudioRoleT audioRole;
    json_object *audioContext = NULL;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:s,s?i,s?s,s?o}", "event_name", &eventName,"audio_role",&audioRole,"media_name",&mediaName,"audio_context",&audioContext);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = event_name:%s audio_role:%d media_name:%s", eventName,audioRole,mediaName);

    // TODO: Post sound event to rendering services

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
