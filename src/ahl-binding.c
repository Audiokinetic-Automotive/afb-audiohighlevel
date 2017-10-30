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

#include "ahl-binding.h"
#include "ahl-apidef.h" // Generated from JSON OpenAPI
#include "wrap-json.h"
#include "ahl-policy.h"
#include "ahl-policy-utils.h"

// Global high-level binding context
AHLCtxT g_AHLCtx; 

// TODO: Helpers that could be common
static EndpointTypeT EndpointTypeToEnum(char * in_pEndpointTypeStr)
{
    if (in_pEndpointTypeStr == NULL) {
        return ENDPOINTTYPE_MAXVALUE;
    }
    else if (strcasecmp(in_pEndpointTypeStr,AHL_ENDPOINTTYPE_SOURCE)==0) {
        return ENDPOINTTYPE_SOURCE;
    }
    else if (strcasecmp(in_pEndpointTypeStr,AHL_ENDPOINTTYPE_SINK)==0) {
        return ENDPOINTTYPE_SINK;
    }
    else 
        return ENDPOINTTYPE_MAXVALUE;
}

static StreamStateT StreamStateToEnum(char * in_pStreamStateStr)
{
    if (in_pStreamStateStr == NULL) {
        return STREAM_STATE_MAXVALUE;
    }
    else if (strcasecmp(in_pStreamStateStr,AHL_STREAM_STATE_IDLE)==0) {
        return STREAM_STATE_IDLE;
    }
    else if (strcasecmp(in_pStreamStateStr,AHL_STREAM_STATE_RUNNING)==0) {
        return STREAM_STATE_RUNNING;
    }
    else if (strcasecmp(in_pStreamStateStr,AHL_STREAM_STATE_PAUSED)==0) {
        return STREAM_STATE_PAUSED;
    }
    else 
        return STREAM_STATE_MAXVALUE;
}

static StreamMuteT StreamMuteToEnum(char * in_pStreamMuteStr)
{
    if (in_pStreamMuteStr == NULL) {
        return STREAM_MUTE_MAXVALUE;
    }
    else if (strcasecmp(in_pStreamMuteStr,AHL_STREAM_UNMUTED)==0) {
        return STREAM_UNMUTED;
    }
    else if (strcasecmp(in_pStreamMuteStr,AHL_STREAM_MUTED)==0) {
        return STREAM_MUTED;
    }
    else 
        return STREAM_MUTE_MAXVALUE;
}

static char * DeviceURITypeEnumToStr(DeviceURITypeT in_eDeviceURIType) {
    switch(in_eDeviceURIType) {
        case DEVICEURITYPE_ALSA_HW:  // Alsa hardware device URI
            return AHL_DEVICEURITYPE_ALSA_HW;
        case DEVICEURITYPE_ALSA_DMIX:    // Alsa Dmix device URI (only for playback devices)
            return AHL_DEVICEURITYPE_ALSA_DMIX;
        case DEVICEURITYPE_ALSA_DSNOOP:  // Alsa DSnoop device URI (only for capture devices)
            return AHL_DEVICEURITYPE_ALSA_DSNOOP;
        case DEVICEURITYPE_ALSA_SOFTVOL: // Alsa softvol device URI
            return AHL_DEVICEURITYPE_ALSA_SOFTVOL;
        case DEVICEURITYPE_ALSA_PLUG:    // Alsa plug device URI
            return AHL_DEVICEURITYPE_ALSA_PLUG;
        case DEVICEURITYPE_ALSA_OTHER:   // Alsa domain URI device of unspecified type
            return AHL_DEVICEURITYPE_ALSA_OTHER;
        case DEVICEURITYPE_NOT_ALSA:     // Unknown (not ALSA domain)
            return AHL_DEVICEURITYPE_NOT_ALSA;
        default:
            return "Unknown";
    }
}

static char * StreamStateEnumToStr(StreamStateT in_eStreamState) {
    switch(in_eStreamState) {
        case STREAM_STATE_IDLE:
            return AHL_STREAM_STATE_IDLE;
        case STREAM_STATE_RUNNING: 
            return AHL_STREAM_STATE_RUNNING;
        case STREAM_STATE_PAUSED: 
            return AHL_STREAM_STATE_PAUSED;
        default:
            return "Unknown";
    }
}

static char * StreamMuteEnumToStr(StreamMuteT in_eStreamMute) {
    switch(in_eStreamMute) {
        case STREAM_UNMUTED: 
            return AHL_STREAM_UNMUTED;
        case STREAM_MUTED: 
            return AHL_STREAM_MUTED;
        default:
            return "Unknown";
    }
}

static void AudioFormatStructToJSON(json_object **audioFormatJ, AudioFormatT * pAudioFormat)
{
    wrap_json_pack(audioFormatJ, "{s:i,s:i,s:i}",
                    "sample_rate", pAudioFormat->sampleRate, 
                    "num_channels", pAudioFormat->numChannels, 
                    "sample_type", pAudioFormat->sampleType);
}

// Package only information that can useful to application clients when selecting endpoint
static void EndpointInfoStructToJSON(json_object **endpointInfoJ, EndpointInfoT * pEndpointInfo)
{
    json_object *formatInfoJ = NULL;
    wrap_json_pack(endpointInfoJ, "{s:i,s:s,s:s,s:s,s:s,s:s,s:s}",
                    "endpoint_id", pEndpointInfo->endpointID, 
                    "endpoint_type", (pEndpointInfo->type == ENDPOINTTYPE_SOURCE) ? AHL_ENDPOINTTYPE_SOURCE : AHL_ENDPOINTTYPE_SINK,
                    "device_name", pEndpointInfo->gsDeviceName, 
                    "display_name", pEndpointInfo->gsDisplayName,
                    "audio_role", pEndpointInfo->pRoleName,
                    "device_domain",pEndpointInfo->gsDeviceDomain,
                    "device_uri_type", DeviceURITypeEnumToStr(pEndpointInfo->deviceURIType));
    AudioFormatStructToJSON(&formatInfoJ,&pEndpointInfo->format);
    json_object_object_add(*endpointInfoJ,"format",formatInfoJ);

    // Properties
    if (pEndpointInfo->pPropTable) {
        json_object *pPropTableJ = json_object_new_array();
        
        GHashTableIter iter;
        gpointer key, value;
        g_hash_table_iter_init (&iter, pEndpointInfo->pPropTable);
        while (g_hash_table_iter_next (&iter, &key, &value))
        {
            if((key!=NULL) && (value!=NULL))
            {
                json_object *pPropertyJ = NULL;
                json_object_get((json_object*)value); // Don't let the framework free our object when the request is done
                wrap_json_pack(&pPropertyJ, "{s:s,s:o}","property_name", (char*)key, "property_value", (json_object*)value);
                json_object_array_add(pPropTableJ, pPropertyJ);
            }
        }
        json_object_object_add(*endpointInfoJ,"properties",pPropTableJ);
    }
}
 
// Package only information that can useful to application clients when opening a stream
static void StreamInfoStructToJSON(json_object **streamInfoJ, StreamInfoT * pStreamInfo)
{
    json_object *endpointInfoJ = NULL;
    EndpointInfoStructToJSON(&endpointInfoJ,pStreamInfo->pEndpointInfo);
    wrap_json_pack(streamInfoJ, "{s:i,s:s,s:s,s:s}", 
        "stream_id", pStreamInfo->streamID,
        "state", StreamStateEnumToStr(pStreamInfo->streamState),
        "mute", StreamMuteEnumToStr(pStreamInfo->streamMute),   
        "device_uri",pStreamInfo->pEndpointInfo->gsDeviceURI); // Need to open a stream to have access to the device URI
    json_object_object_add(*streamInfoJ,"endpoint_info",endpointInfoJ);
}

static streamID_t CreateNewStreamID()
{
    streamID_t newID = g_AHLCtx.nextStreamID;
    g_AHLCtx.nextStreamID++;
    return newID;
}

static EndpointInfoT * GetEndpointInfoWithRole(endpointID_t in_endpointID, EndpointTypeT in_endpointType, RoleInfoT * in_pRole)
{
    EndpointInfoT * pEndpointInfo = NULL;
    GPtrArray * pDeviceArray = NULL;
    if (in_endpointType == ENDPOINTTYPE_SOURCE){
        pDeviceArray = in_pRole->pSourceEndpoints;
    }
    else { 
        pDeviceArray = in_pRole->pSinkEndpoints;
    }
    g_assert_nonnull(pDeviceArray);

    for (int j = 0; j < pDeviceArray->len; j++) {
        EndpointInfoT * pCurEndpointInfo = g_ptr_array_index(pDeviceArray,j);
        g_assert_nonnull(pCurEndpointInfo);
        if (pCurEndpointInfo->endpointID == in_endpointID) {
            pEndpointInfo = pCurEndpointInfo;
            break;
        }
    }

    return pEndpointInfo;
}

static int ReplaceEndpointInfoWithRole(endpointID_t in_endpointID, EndpointTypeT in_endpointType, RoleInfoT * in_pRole, EndpointInfoT * in_pNewEndpoint)
{
    GPtrArray * pDeviceArray = NULL;
    if (in_endpointType == ENDPOINTTYPE_SOURCE){
        pDeviceArray = in_pRole->pSourceEndpoints;
    }
    else { 
        pDeviceArray = in_pRole->pSinkEndpoints;
    }
    g_assert_nonnull(pDeviceArray);

    for (int j = 0; j < pDeviceArray->len; j++) {
        EndpointInfoT * pCurEndpointInfo = g_ptr_array_index(pDeviceArray,j);
        g_assert_nonnull(pCurEndpointInfo);
        if (pCurEndpointInfo->endpointID == in_endpointID) {            
            g_ptr_array_insert(pDeviceArray,j,in_pNewEndpoint);
            g_ptr_array_remove_index(pDeviceArray,j+1);
            TermEndpointInfo(pCurEndpointInfo);
            // GLib automatically frees item upon array removal
            return AHL_SUCCESS;
        }
    }

    return AHL_FAIL;
}

static EndpointInfoT * GetEndpointInfo(endpointID_t in_endpointID, EndpointTypeT in_endpointType)
{
    EndpointInfoT * pEndpointInfo = NULL;

    GHashTableIter iter;
    gpointer key, value;      
    g_hash_table_iter_init (&iter, g_AHLCtx.policyCtx.pRoleInfo);
    while (pEndpointInfo == NULL && g_hash_table_iter_next (&iter, &key, &value))
    {
        RoleInfoT * pRoleInfo = (RoleInfoT*)value;
        pEndpointInfo = GetEndpointInfoWithRole(in_endpointID,in_endpointType,pRoleInfo);
    }

    return pEndpointInfo;
}

static StreamInfoT * GetStream(streamID_t in_streamID)
{
    if (g_AHLCtx.policyCtx.pStreams == NULL)
        return NULL;
    
    return g_hash_table_lookup(g_AHLCtx.policyCtx.pStreams,GINT_TO_POINTER(&in_streamID));
}

static RoleInfoT * GetRole(char * in_pAudioRoleName)
{
    if (g_AHLCtx.policyCtx.pRoleInfo == NULL)
        return NULL;
    
    return g_hash_table_lookup(g_AHLCtx.policyCtx.pRoleInfo,in_pAudioRoleName);
}

static AHLClientCtxT * AllocateClientContext()
{
    AHLClientCtxT * pClientCtx = malloc(sizeof(AHLClientCtxT));
    pClientCtx->pStreamAccessList = g_array_new(FALSE, TRUE, sizeof(streamID_t));
    return pClientCtx;
}

static void TerminateClientContext(void * ptr)
{
    AHLClientCtxT * pClientCtx = (AHLClientCtxT *) ptr;
    if (pClientCtx != NULL) {

        // Avoid having policy in bad state if client loses WS connection (e.g. app termination without close stream)
        // Force close streams in those cases.
 
        if (pClientCtx->pStreamAccessList != NULL) {
#ifndef AHL_DISCONNECT_POLICY 
            for (int i = 0; i < pClientCtx->pStreamAccessList->len; i++)
            {
                streamID_t streamID = g_array_index(pClientCtx->pStreamAccessList,streamID_t,i);
                // Call policy to verify whether creating a new audio stream is allowed in current context and possibly take other actions
                StreamInfoT * pStreamInfo = GetStream(streamID);
                if (pStreamInfo == NULL) {
                    AFB_ERROR("Specified stream not currently active stream_id -> %d",streamID);
                    return;
                }
                
                json_object *pPolicyStreamJ = NULL;
                int err = PolicyStreamStructToJSON(pStreamInfo, &pPolicyStreamJ);
                if (err == AHL_POLICY_UTIL_FAIL)
                {
                    AFB_ERROR("Audio policy violation, Unable to get JSON object for Policy_CloseStream");
                    return;
                } 
                Policy_CloseStream(pPolicyStreamJ);
            }  
#endif
            g_array_free( pClientCtx->pStreamAccessList, TRUE);
            pClientCtx->pStreamAccessList = NULL;
        }
        free(pClientCtx);
    }
}

static int CheckStreamAccessControl(AHLClientCtxT * pClientCtx, streamID_t streamID)
{
    int iAccessControl = AHL_ACCESS_CONTROL_DENIED;
    if (pClientCtx && pClientCtx->pStreamAccessList) {
        for (int i = 0; i < pClientCtx->pStreamAccessList->len ; i++) {
            streamID_t iID = g_array_index(pClientCtx->pStreamAccessList,streamID_t,i);
            if (iID == streamID) {
                iAccessControl = AHL_ACCESS_CONTROL_GRANTED;
            }    
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
        return err;
    }

    g_AHLCtx.policyCtx.volumeEvent = afb_daemon_make_event(AHL_ENDPOINT_VOLUME_EVENT);
    err = !afb_event_is_valid(g_AHLCtx.policyCtx.volumeEvent);
    if (err) {
        AFB_ERROR("Could not create endpoint volume change event");
        return err;
    }

    g_AHLCtx.policyCtx.postActionEvent = afb_daemon_make_event(AHL_POST_ACTION_EVENT);
    err = !afb_event_is_valid(g_AHLCtx.policyCtx.postActionEvent);
    if (err) {
        AFB_ERROR("Could not create post action event call event");
        return err;
    }

    return err;
}

static void AhlBindingTerm()
{
#ifndef AHL_DISCONNECT_POLICY    
    // Policy termination
    Policy_Term();
#endif

    // Roles
    if (g_AHLCtx.policyCtx.pRoleInfo != NULL) {
        GHashTableIter iter;
        gpointer key, value;
        g_hash_table_iter_init(&iter, g_AHLCtx.policyCtx.pRoleInfo);
        while (g_hash_table_iter_next (&iter, &key, &value))
        {
            RoleInfoT * pRole = (RoleInfoT *)value;
            if (pRole)
            {
                if(pRole->pRoleName) {
                    g_free(pRole->pRoleName);
                    pRole->pRoleName = NULL;
                }
                // Actions
                if (pRole->pActionList) {
                    for (int i = 0; i < pRole->pActionList->len; i++)
                    {
                        g_ptr_array_remove_index( pRole->pActionList, i ); // Free char * is called by GLib
                    }
                }
                // Source endpoints
                if (pRole->pSourceEndpoints) {
                    for (int i = 0; i < pRole->pSourceEndpoints->len; i++)
                    {
                        EndpointInfoT * pEndpoint = g_ptr_array_remove_index( pRole->pSourceEndpoints, i );  // Free endpoint * is called by GLib
                        if (pEndpoint) {
                            TermEndpointInfo(pEndpoint);
                        }
                    }
                }
                // Sink endpoints
                if (pRole->pSinkEndpoints) {
                    for (int i = 0; i < pRole->pSinkEndpoints->len; i++)
                    {
                        EndpointInfoT * pEndpoint = g_ptr_array_remove_index( pRole->pSinkEndpoints, i ); // Free endpoint * is called by GLib
                        if (pEndpoint) {
                            TermEndpointInfo(pEndpoint);
                        }
                    }
                }
                free(pRole);
            }
        }
        g_hash_table_remove_all(g_AHLCtx.policyCtx.pRoleInfo);
        g_hash_table_destroy(g_AHLCtx.policyCtx.pRoleInfo);
        g_AHLCtx.policyCtx.pRoleInfo = NULL;
    }
   
   if (g_AHLCtx.policyCtx.pStreams) {
        GHashTableIter iter;
        gpointer key, value;
        g_hash_table_iter_init (&iter, g_AHLCtx.policyCtx.pStreams);
        while (g_hash_table_iter_next (&iter, &key, &value))
        {
            if (value)
                free(value);
        } 
        g_hash_table_remove_all(g_AHLCtx.policyCtx.pStreams);
        g_hash_table_destroy(g_AHLCtx.policyCtx.pStreams);
        g_AHLCtx.policyCtx.pStreams = NULL;
   }

    if (g_AHLCtx.policyCtx.pHALList) {
        g_ptr_array_free(g_AHLCtx.policyCtx.pHALList,TRUE);
        g_AHLCtx.policyCtx.pHALList = NULL;
    }

    AFB_INFO("Audio high-level binding termination success");
}

// Binding initialization
PUBLIC int AhlBindingInit()
{
    int err = 0;

    memset(&g_AHLCtx,0,sizeof(g_AHLCtx));
    
    // Register exit function
    atexit(AhlBindingTerm);

    //Create AGL Events
    err=CreateEvents();
    if(err)
    {
        //Error messages already reported inside CreateEvents
        return err;
    }

    // Parse high-level binding JSON configuration file (will build device lists)
    err = ParseHLBConfig();
    if(err)
    {
        //Error messages already reported inside ParseHLBConfig
        return err;
    }
    
#ifndef AHL_DISCONNECT_POLICY  
    // Policy initialization
    err = Policy_Init();
    if(err == AHL_POLICY_REJECT)
    {
        //Error messages already reported inside PolicyInit
        return err;        
    }

    // for all audio Roles
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init (&iter, g_AHLCtx.policyCtx.pRoleInfo);
    while (g_hash_table_iter_next (&iter, &key, &value))
    {
        RoleInfoT * pRoleInfo = (RoleInfoT*)value;
        if (pRoleInfo->pSourceEndpoints){
            // for all source endpoints
            for (int j = 0; j < pRoleInfo->pSourceEndpoints->len; j++) {
                EndpointInfoT * pCurEndpointInfo = g_ptr_array_index(pRoleInfo->pSourceEndpoints,j);
                g_assert_nonnull(pCurEndpointInfo);
                json_object *pPolicyEndpointJ = NULL;
                err = PolicyEndpointStructToJSON(pCurEndpointInfo, &pPolicyEndpointJ);
                if (err == AHL_POLICY_UTIL_FAIL)
                {
                    AFB_ERROR("Unable to Create Endpoint Json object error:%s ",wrap_json_get_error_string(err));
                    return err;    
                }
                else
                {
                    err = Policy_Endpoint_Init(pPolicyEndpointJ);
                    if (err == AHL_POLICY_REJECT) {
                        AFB_ERROR("Policy endpoint properties initalization failed for endpoint_id :%d type:%d",pCurEndpointInfo->endpointID, pCurEndpointInfo->type);
                    }                    
                    //free pPolicyEndpointJ
                    json_object_put(pPolicyEndpointJ);
                }                                 
            }
        }
        if (pRoleInfo->pSinkEndpoints){
            // for all sink endpoints
            for (int j = 0; j < pRoleInfo->pSinkEndpoints->len; j++) {
                EndpointInfoT * pCurEndpointInfo = g_ptr_array_index(pRoleInfo->pSinkEndpoints,j);
                g_assert_nonnull(pCurEndpointInfo);
                json_object *pPolicyEndpointJ = NULL;
                err = PolicyEndpointStructToJSON(pCurEndpointInfo, &pPolicyEndpointJ);
                if (err == AHL_POLICY_UTIL_FAIL)
                {
                    AFB_ERROR("Unable to Create Endpoint Json object error:%s ",wrap_json_get_error_string(err));
                    return err;
                }
                else
                {
                    err = Policy_Endpoint_Init(pPolicyEndpointJ);
                    if (err== AHL_POLICY_REJECT) {
                        AFB_ERROR("Policy endpoint properties initalization failed for endpoint_id :%d type:%d",pCurEndpointInfo->endpointID, pCurEndpointInfo->type);                        
                    }
                    //free pPolicyEndpointJ
                    json_object_put(pPolicyEndpointJ);
                } 

            }
        }
    }    
#endif   

    // Initialize list of active streams
    g_AHLCtx.policyCtx.pStreams = g_hash_table_new(g_int_hash, g_int_equal);
    if(g_AHLCtx.policyCtx.pStreams == NULL)
    {
        AFB_ERROR("Unable to create Active Stream List");
        return err;
    }

    // TODO: Use AGL persistence framework to retrieve and set initial volumes/properties

    AFB_DEBUG("Audio high-level Binding success");
    return err;
}

PUBLIC void AhlOnEvent(const char *evtname, json_object *eventJ)
{
    AFB_DEBUG("AHL received event %s", evtname);

    // TODO: Handle event from the policy to update internal information (currently not possible since within the same binding)
    
#ifndef AHL_DISCONNECT_POLICY  
    // Temp: currently forward to policy to handle events  (they will be received directly when disconnected into separate binding)
    Policy_OnEvent(evtname, eventJ);
#endif
}

PUBLIC void audiohlapi_get_sources(struct afb_req req)
{
    json_object *devicesJ = NULL;
    json_object *deviceJ = NULL;
    json_object *queryJ = NULL;
    char * audioRole = NULL;

    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:s}", "audio_role", &audioRole);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }

    AFB_DEBUG("Filtering devices according to specified audio role=%s", audioRole);

    RoleInfoT * pRole = GetRole(audioRole);
    if ( pRole == NULL )
    {
        afb_req_fail_f(req, "Invalid arguments", "Requested audio role does not exist in current configuration -> %s", json_object_get_string(queryJ));
        return;
    }
    else
    {
        devicesJ = json_object_new_array();
        GPtrArray * pDeviceArray = pRole->pSourceEndpoints;
        if (pDeviceArray) {
            int iNumberDevices = pDeviceArray->len;
            for ( int j = 0 ; j < iNumberDevices; j++)
            {
                EndpointInfoT * pEndpointInfo = g_ptr_array_index(pDeviceArray,j);
                if (pEndpointInfo) {
                    EndpointInfoStructToJSON(&deviceJ, pEndpointInfo);
                    json_object_array_add(devicesJ, deviceJ);
                }
            }
        }
    } 

    afb_req_success(req, devicesJ, "List of sources");
}

PUBLIC void audiohlapi_get_sinks(struct afb_req req)
{
    json_object *devicesJ = NULL;
    json_object *deviceJ = NULL;
    json_object *queryJ = NULL;
    char * audioRole = NULL;

    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:s}", "audio_role", &audioRole);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }

    AFB_DEBUG("Filtering devices according to specified audio role=%s", audioRole);

    RoleInfoT * pRole = GetRole(audioRole);
    if ( pRole == NULL )
    {
        afb_req_fail_f(req, "Invalid arguments", "Requested audio role does not exist in current configuration -> %s", json_object_get_string(queryJ));
        return;
    }
    else
    {
        devicesJ = json_object_new_array();
        GPtrArray * pDeviceArray = pRole->pSinkEndpoints;
        if (pDeviceArray) {
            int iNumberDevices = pDeviceArray->len;
            for ( int j = 0 ; j < iNumberDevices; j++)
            {
                EndpointInfoT * pEndpointInfo = g_ptr_array_index(pDeviceArray,j);
                EndpointInfoStructToJSON(&deviceJ, pEndpointInfo);
                json_object_array_add(devicesJ, deviceJ);
            }
        }
    } 

    afb_req_success(req, devicesJ, "List of sinks");
}

PUBLIC void audiohlapi_stream_open(struct afb_req req)
{
    json_object *streamInfoJ = NULL;
    StreamInfoT * pStreamInfo = NULL;
    json_object *queryJ = NULL;
    char * audioRole = NULL;
    char * endpointTypeStr = NULL;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    endpointID_t endpointID = AHL_UNDEFINED;
    EndpointInfoT * pEndpointInfo = NULL;
    EndpointSelectionModeT endpointSelMode = ENDPOINTSELMODEMAXVALUE;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:s,s:s,s?i}", "audio_role", &audioRole, "endpoint_type", &endpointTypeStr, "endpoint_id", &endpointID);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = audio_role:%s endpoint_type:%s endpoint_id:%d", audioRole,endpointTypeStr,endpointID);
    endpointType = EndpointTypeToEnum(endpointTypeStr);

    // Check if there is already an existing context for this client
    AHLClientCtxT * pClientCtx = afb_req_context_get(req); // Retrieve client-specific data structure
    if (pClientCtx == NULL)
    {
        pClientCtx = AllocateClientContext();
        afb_req_context_set(req, pClientCtx, TerminateClientContext);
    }

    RoleInfoT * pRole = GetRole(audioRole);
    if ( pRole == NULL )
    {
        afb_req_fail_f(req, "Invalid audio role", "Audio role was not found in configuration -> %s",audioRole);
        return;
    }

    GPtrArray * pDeviceArray = NULL;
    if (endpointType == ENDPOINTTYPE_SOURCE){
        pDeviceArray = pRole->pSourceEndpoints;
    }
    else{
        pDeviceArray = pRole->pSinkEndpoints;
    }
    if (pDeviceArray == NULL || pDeviceArray->len == 0) {
        afb_req_fail_f(req, "No available devices", "No available devices for role:%s and device type:%s",audioRole,endpointTypeStr);
        return;
    }

    if (endpointID == AHL_UNDEFINED)
    {
        // Assign a device based on configuration priority (first in the list for requested role and endpoint type)
        pEndpointInfo = g_ptr_array_index(pDeviceArray,0);
        endpointSelMode = ENDPOINTSELMODE_AUTO;
        
    }
    else{
        endpointSelMode = ENDPOINTSELMODE_MANUAL;
        // Find specified endpoint ID in list of devices
        int iNumberDevices = pDeviceArray->len;
        for ( int j = 0 ; j < iNumberDevices; j++)
        {
            pEndpointInfo = g_ptr_array_index(pDeviceArray,j);
            if (pEndpointInfo && pEndpointInfo->endpointID == endpointID) {
                break;
            }
            pEndpointInfo = NULL;
        }

    }

    if (pEndpointInfo == NULL) {
        afb_req_fail_f(req, "Endpoint not available", "Specified endpoint not available for role:%s and device type:%d endpoint id %d",audioRole,endpointType,endpointID);
        return;
    }

    pStreamInfo = (StreamInfoT*) malloc(sizeof(StreamInfoT));
    memset(pStreamInfo,0,sizeof(StreamInfoT));

    // Create stream
    pStreamInfo->streamID = CreateNewStreamID(); // create new ID
    pStreamInfo->streamState = STREAM_STATE_IDLE;
    pStreamInfo->streamMute = STREAM_UNMUTED;
    pStreamInfo->pEndpointInfo = pEndpointInfo;
    pStreamInfo->endpointSelMode = endpointSelMode;
    // Directly from role config for now, but could be programmatically overriden in the future
    pStreamInfo->pRoleName = pRole->pRoleName;         
    pStreamInfo->iPriority = pRole->iPriority;   
    pStreamInfo->eInterruptBehavior = pRole->eInterruptBehavior;

#ifndef AHL_DISCONNECT_POLICY  
    // Call policy to verify whether creating a new audio stream is allowed in current context and possibly take other actions
    json_object *pPolicyStreamJ = NULL;
    err = PolicyStreamStructToJSON(pStreamInfo, &pPolicyStreamJ);
    if (err == AHL_POLICY_UTIL_FAIL)
    {
        afb_req_fail(req, "Audio policy violation", "Unable to get JSON object for Policy_OpenStream");
        return;
    } 
    
    int policyAllowed = Policy_OpenStream(pPolicyStreamJ);    
    if (policyAllowed == AHL_POLICY_REJECT)
    {
        afb_req_fail(req, "Audio policy violation", "Open stream not allowed in current context");
        return;
    } 
#endif   

    char streamEventName[128];
    snprintf(streamEventName,128,"ahl_streamstate_%d",pStreamInfo->streamID);
    
    pStreamInfo->streamStateEvent = afb_daemon_make_event(streamEventName);
    err = !afb_event_is_valid(pStreamInfo->streamStateEvent);
    if (err) {
        afb_req_fail(req, "Stream event creation failure", "Could not create stream specific state change event");
        return;
    }

    err = afb_req_subscribe(req,pStreamInfo->streamStateEvent);
    if (err) {
        afb_req_fail(req, "Stream event subscription failure", "Could not subscribe to stream specific state change event");
        return;
    }

    // Add to client context stream ID access rights
    g_array_append_val(pClientCtx->pStreamAccessList, pStreamInfo->streamID);

    // Push stream on active stream list
    if (g_AHLCtx.policyCtx.pStreams)
        g_hash_table_insert( g_AHLCtx.policyCtx.pStreams, GINT_TO_POINTER(&pStreamInfo->streamID), pStreamInfo );

    StreamInfoStructToJSON(&streamInfoJ,pStreamInfo);

    afb_req_success(req, streamInfoJ, "Stream info structure");
}

PUBLIC void audiohlapi_stream_close(struct afb_req req)
{
    json_object *queryJ = NULL;
    streamID_t streamID = AHL_UNDEFINED;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i}", "stream_id", &streamID);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = stream_id:%d", streamID);

    
    StreamInfoT * pStreamInfo = GetStream(streamID);
    if (pStreamInfo == NULL) {
        afb_req_fail_f(req, "Stream not found", "Specified stream not currently active stream_id -> %d",streamID);
        return;
    }

    // Check if there is already an existing context for this client
    AHLClientCtxT * pClientCtx = afb_req_context_get(req); // Retrieve client-specific data structure
    if (pClientCtx == NULL)
    {
        afb_req_fail(req, "Bad state", "No client context associated with the request (is there an opened stream by this client?)");
        return;
    }

    // Verify that this client can control the stream
    int iStreamAccessControl = CheckStreamAccessControl( pClientCtx, streamID );
    if (iStreamAccessControl == AHL_ACCESS_CONTROL_DENIED)
    {
        afb_req_fail(req, "Access control denied", "Close stream not allowed in current client context");
        return;
    }

#ifndef AHL_DISCONNECT_POLICY  
    json_object *pPolicyStreamJ = NULL;
    err = PolicyStreamStructToJSON(pStreamInfo, &pPolicyStreamJ);
    if (err == AHL_POLICY_UTIL_FAIL)
    {
        afb_req_fail(req, "Audio policy violation", "Unable to get JSON object for Policy_CloseStream");
        return;
    } 
    // Call policy to verify whether creating a new audio stream is allowed in current context and possibly take other actions
    int policyAllowed = Policy_CloseStream(pPolicyStreamJ);
    if (policyAllowed == AHL_POLICY_REJECT)
    {
        afb_req_fail(req, "Audio policy violation", "Close stream not allowed in current context");
        return;
    }
#endif

    // Unsubscribe client from stream events
    char streamEventName[128];
    snprintf(streamEventName,128,"ahl_streamstate_%d",streamID);
    int iValid = afb_event_is_valid(pStreamInfo->streamStateEvent);
    if (iValid) {
        err = afb_req_unsubscribe(req,pStreamInfo->streamStateEvent);
        if (err) {
            afb_req_fail(req, "Stream event subscription failure", "Could not unsubscribe to stream specific state change event");
            return;
        }
    }

    // Remove from stream list (if present)
    if (g_AHLCtx.policyCtx.pStreams)
        g_hash_table_remove(g_AHLCtx.policyCtx.pStreams,GINT_TO_POINTER(&pStreamInfo->streamID));
    free(pStreamInfo);
    pStreamInfo = NULL;

    // Find index for cases where there are multiple streams per client
    // Remove from client context stream ID and endpoint ID access rights
    if (pClientCtx->pStreamAccessList) {
        for (int i = 0; i < pClientCtx->pStreamAccessList->len ; i++) {
            streamID_t iID = g_array_index(pClientCtx->pStreamAccessList,streamID_t,i);
            if (iID == streamID) {
                g_array_remove_index(pClientCtx->pStreamAccessList, i);
            }    
        }

        if (pClientCtx->pStreamAccessList->len == 0) {
            // If no more streams/endpoints owner, clear session
            afb_req_context_clear(req);
        }
    }

    afb_req_success(req, NULL, "Stream close completed");
}

 PUBLIC void audiohlapi_set_stream_state(struct afb_req req)
 {
    json_object *queryJ = NULL;
    streamID_t streamID = AHL_UNDEFINED;
    char * streamStateStr = NULL;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:s}", "stream_id", &streamID,"state",&streamStateStr);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = stream_id:%d, state:%s", streamID,streamStateStr);
    
    StreamInfoT * pStreamInfo = GetStream(streamID);
    if (pStreamInfo == NULL) {
        afb_req_fail_f(req, "Stream not found", "Specified stream not found stream_id -> %d",streamID);
        return;
    }

    // Check if there is already an existing context for this client
    AHLClientCtxT * pClientCtx = afb_req_context_get(req); // Retrieve client-specific data structure
    if (pClientCtx == NULL)
    {
        afb_req_fail(req, "Bad state", "No client context associated with the request (is there an opened stream by this client?)");
        return;
    }

    // Verify that this client can control the stream
    int iStreamAccessControl = CheckStreamAccessControl( pClientCtx, streamID );
    if (iStreamAccessControl == AHL_ACCESS_CONTROL_DENIED)
    {
        afb_req_fail(req, "Access control denied", "Set stream state not allowed in current client context");
        return;
    }

    StreamStateT streamState = StreamStateToEnum(streamStateStr);
#ifndef AHL_DISCONNECT_POLICY  
    json_object *pPolicyStreamJ = NULL;
    err = PolicyStreamStructToJSON(pStreamInfo, &pPolicyStreamJ);
    if (err == AHL_POLICY_UTIL_FAIL)
    {
        afb_req_fail(req, "Audio policy violation", "Unable to get JSON object for Policy_SetStreamState");
        return;
    } 

    json_object *paramJ= json_object_new_int(streamState);
    json_object_object_add(pPolicyStreamJ, "arg_stream_state", paramJ);

    int policyAllowed = Policy_SetStreamState(pPolicyStreamJ);    
    if (policyAllowed == AHL_POLICY_REJECT)
    {
        afb_req_fail(req, "Audio policy violation", "Change stream state not allowed in current context");
        return; 
    }
#else
    // Simulate that policy returns target state (accepted)
    pStreamInfo->streamState = streamState;
#endif

    afb_req_success(req, NULL, "Set stream state");
 }

 PUBLIC void audiohlapi_set_stream_mute(struct afb_req req)
 {
    json_object *queryJ = NULL;
    streamID_t streamID = AHL_UNDEFINED;
    char * pMuteStr = NULL;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:i,s:s}", "stream_id", &streamID,"mute",&pMuteStr);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = stream_id:%d, mute:%s", streamID,pMuteStr);

    StreamInfoT * pStreamInfo = GetStream(streamID);
    if (pStreamInfo == NULL) {
        afb_req_fail_f(req, "Stream not found", "Specified stream not currently active stream_id -> %d",streamID);
        return;
    }

    // Check if there is already an existing context for this client
    AHLClientCtxT * pClientCtx = afb_req_context_get(req); // Retrieve client-specific data structure
    if (pClientCtx == NULL)
    {
        afb_req_fail(req, "Bad state", "No client context associated with the request (is there an opened stream by this client?)");
        return;
    }

    // Verify that this client can control the stream
    int iStreamAccessControl = CheckStreamAccessControl( pClientCtx, streamID );
    if (iStreamAccessControl == AHL_ACCESS_CONTROL_DENIED)
    {
        afb_req_fail(req, "Access control denied", "Set stream mute state not allowed in current client context");
        return;
    }


    StreamMuteT muteState = StreamMuteToEnum(pMuteStr);
#ifndef AHL_DISCONNECT_POLICY 
    json_object *pPolicyStreamJ = NULL;
    err = PolicyStreamStructToJSON(pStreamInfo, &pPolicyStreamJ);
    if (err == AHL_POLICY_UTIL_FAIL)
    {
        afb_req_fail(req, "Audio policy violation", "Unable to get JSON object for Policy_SetStreamMute");
        return;
    } 

    json_object *paramJ= json_object_new_int(muteState);
    json_object_object_add(pPolicyStreamJ, "mute_state", paramJ);

    int policyAllowed = Policy_SetStreamMute(pPolicyStreamJ);    
    if (policyAllowed == AHL_POLICY_REJECT)
    {
        afb_req_fail(req, "Audio policy violation", "Mute stream not allowed in current context");
        return;
    }
#else
    // Simulate that policy returns target state (accepted)
    pStreamInfo->streamMute = muteState;
#endif

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

    StreamInfoT * pStreamInfo = GetStream(streamID);
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
    char * pEndpointTypeStr = NULL;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    char * volumeStr = NULL;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:s,s:i,s:s}", "endpoint_type", &pEndpointTypeStr,"endpoint_id",&endpointID,"volume",&volumeStr);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%s endpoint_id:%d volume:%s", pEndpointTypeStr,endpointID,volumeStr);
    endpointType = EndpointTypeToEnum(pEndpointTypeStr);

    EndpointInfoT * pEndpointInfo = GetEndpointInfo(endpointID,endpointType);
    if (pEndpointInfo == NULL)
    {
        afb_req_fail_f(req, "Endpoint not found", "Endpoint information not found for id:%d type%d",endpointID,endpointType);
        return;
    }

#ifndef AHL_DISCONNECT_POLICY
    json_object *pPolicyEndpointJ = NULL;
    err = PolicyEndpointStructToJSON(pEndpointInfo, &pPolicyEndpointJ);
    if (err == AHL_POLICY_UTIL_FAIL)
    {
        afb_req_fail(req, "Audio policy violation", "Unable to get JSON object for Policy_SetVolume");
        return;
    } 

    json_object *paramJ= json_object_new_string(volumeStr);
    json_object_object_add(pPolicyEndpointJ, "arg_volume", paramJ);

    int policyAllowed = Policy_SetVolume(pPolicyEndpointJ);
    if (!policyAllowed)
    {
        afb_req_fail(req, "Audio policy violation", "Set volume not allowed in current context");
        return;
    }
#else
    // Simulate that policy returns target state (accepted)
    pEndpointInfo->iVolume = atoi(volumeStr);
#endif
 
    afb_req_success(req, NULL, "Set volume completed");
}

PUBLIC void audiohlapi_get_volume(struct afb_req req)
{
    json_object *queryJ = NULL;
    endpointID_t endpointID = AHL_UNDEFINED;
    char * pEndpointTypeStr = NULL;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    json_object * volumeJ = NULL;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:s,s:i}", "endpoint_type", &pEndpointTypeStr,"endpoint_id",&endpointID);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%s endpoint_id:%d", pEndpointTypeStr,endpointID);
    endpointType = EndpointTypeToEnum(pEndpointTypeStr);

    EndpointInfoT * pEndpointInfo = GetEndpointInfo(endpointID,endpointType);
    if (pEndpointInfo == NULL)
    {
        afb_req_fail_f(req, "Endpoint not found", "Endpoint information not found for id:%d type%d",endpointID,endpointType);
        return;
    }

    volumeJ = json_object_new_int(pEndpointInfo->iVolume);

    afb_req_success(req, volumeJ, "Retrieved volume value");
}

// Properties 
PUBLIC void audiohlapi_get_endpoint_info(struct afb_req req)
{
    json_object *queryJ = NULL;
    endpointID_t endpointID = AHL_UNDEFINED;
    char * pEndpointTypeStr = NULL;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:s,s:i}", "endpoint_type", &pEndpointTypeStr,"endpoint_id",&endpointID);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%s endpoint_id:%d", pEndpointTypeStr,endpointID);
    endpointType = EndpointTypeToEnum(pEndpointTypeStr);

    EndpointInfoT * pEndpointInfo = GetEndpointInfo(endpointID,endpointType);
    if (pEndpointInfo == NULL)
    {
        afb_req_fail_f(req, "Endpoint not found", "Endpoint information not found for id:%d type%d",endpointID,endpointType);
        return;
    }

    json_object *endpointInfoJ = NULL;
    EndpointInfoStructToJSON(&endpointInfoJ,pEndpointInfo);

    afb_req_success(req, endpointInfoJ, "Retrieved endpoint information and properties");
}

PUBLIC void audiohlapi_set_property(struct afb_req req)
{
    json_object *queryJ = NULL;
    endpointID_t endpointID = AHL_UNDEFINED;
    char * pEndpointTypeStr = NULL;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    char * propertyName = NULL;
    json_object * propValueJ = NULL;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:s,s:i,s:s,s:o}", "endpoint_type", &pEndpointTypeStr,"endpoint_id",&endpointID,"property_name",&propertyName,"value",&propValueJ);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%s endpoint_id:%d property_name:%s", pEndpointTypeStr,endpointID,propertyName);
    endpointType = EndpointTypeToEnum(pEndpointTypeStr);

    EndpointInfoT * pEndpointInfo = GetEndpointInfo(endpointID,endpointType);
    if (pEndpointInfo == NULL)
    {
        afb_req_fail_f(req, "Endpoint not found", "Endpoint information not found for id:%d type%d",endpointID,endpointType);
        return;
    }

#ifndef AHL_DISCONNECT_POLICY  
    json_object *pPolicyEndpointJ = NULL;
    err = PolicyEndpointStructToJSON(pEndpointInfo, &pPolicyEndpointJ);
    if (err == AHL_POLICY_UTIL_FAIL)
    {
        afb_req_fail(req, "Audio policy violation", "Unable to get JSON object for Policy_SetVolume");
        return;
    } 

    json_object *paramJ= json_object_new_string(propertyName);
    json_object_object_add(pPolicyEndpointJ, "arg_property_name", paramJ);
    json_object_object_add(pPolicyEndpointJ, "arg_property_value", propValueJ);

    // Call policy to allow custom policy actions in current context
    int policyAllowed = Policy_SetProperty(pPolicyEndpointJ);     
    if (!policyAllowed)
    {
        afb_req_fail(req, "Audio policy violation", "Set endpoint property not allowed in current context");
        return;
    }
#else
    // Simulate that policy returns target state (accepted)
    if (pEndpointInfo->pPropTable)
        g_hash_table_insert(pEndpointInfo->pPropTable, propertyName, propValueJ);
#endif

    //afb_event_push(g_AHLCtx.policyCtx.propertyEvent,queryJ);

    afb_req_success(req, NULL, "Set property completed");
}

PUBLIC void audiohlapi_get_property(struct afb_req req)
{
    json_object *queryJ = NULL;
    endpointID_t endpointID = AHL_UNDEFINED;
    char * pEndpointTypeStr = NULL;
    EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
    char * propertyName = NULL;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:s,s:i,s:s}", "endpoint_type", &pEndpointTypeStr,"endpoint_id",&endpointID,"property_name",&propertyName);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = endpoint_type:%s endpoint_id:%d property_name:%s", pEndpointTypeStr,endpointID,propertyName);
    endpointType = EndpointTypeToEnum(pEndpointTypeStr);

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
    //AFB_WARNING("properties %s", json_object_get_string(propertyValJ));
    //json_object_get(propertyValJ); // Increase ref count so that framework does not free our JSON object

    afb_req_success(req, propertyValJ, "Retrieved property value");
}

// Audio related events

PUBLIC void audiohlapi_get_list_actions(struct afb_req req)
{
    json_object *queryJ = NULL;
    char * audioRole = NULL;
    json_object * roleActionsJ = NULL;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:s}", "audio_role",&audioRole);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = audio_role:%s",audioRole);

    // Build and return list of actions for specific audio role
    RoleInfoT * pRole = GetRole(audioRole);
    if ( pRole == NULL )
    {
        afb_req_fail_f(req, "Invalid audio role", "Audio role was not found in configuration -> %s",audioRole);
        return;
    }

    roleActionsJ = json_object_new_array();
    if (pRole->pActionList) {
        int iNumberActions = pRole->pActionList->len;
        for ( int i = 0 ; i < iNumberActions; i++)
        {
            char * pActionName = g_ptr_array_index(pRole->pActionList,i);
            json_object * actionJ = json_object_new_string(pActionName);
            json_object_array_add(roleActionsJ, actionJ);
        }
    }
    
    afb_req_success(req, roleActionsJ, "Retrieved action list for audio role");
}

PUBLIC void audiohlapi_post_action(struct afb_req req)
{
    json_object *queryJ = NULL;
    char * actionName = NULL;  
    char * audioRole = NULL;
    char * mediaName = NULL;
    json_object *actionContext = NULL;
    
    queryJ = afb_req_json(req);
    int err = wrap_json_unpack(queryJ, "{s:s,s:s,s?s,s?o}", "action_name", &actionName,"audio_role",&audioRole,"media_name",&mediaName,"action_context",&actionContext);
    if (err) {
        afb_req_fail_f(req, "Invalid arguments", "Args not a valid json object query=%s", json_object_get_string(queryJ));
        return;
    }
    AFB_DEBUG("Parsed input arguments = action_name:%s audio_role:%s", actionName,audioRole);

    // Verify if known action for audio role
    RoleInfoT * pRole = GetRole(audioRole);
    if ( pRole == NULL )
    {
        afb_req_fail_f(req, "Invalid audio role", "Audio role was not found in configuration -> %s",audioRole);
        return;
    }

    // Check to find specific action
    int iActionFound = 0;
    if (pRole->pActionList) {
        int iNumberActions = pRole->pActionList->len;
        char * pTargetActionName = NULL;
        for ( int i = 0 ; i < iNumberActions; i++)
        {
            pTargetActionName = g_ptr_array_index(pRole->pActionList,i);
            if ( strcasecmp(pTargetActionName,actionName)==0) {
                iActionFound = 1;
                break;
            }
        }
    }

    if (!iActionFound) {
        afb_req_fail_f(req, "Event not found for audio role", "Event -> %s not found for role:%s",actionName,audioRole);
        return;
    }

#ifndef AHL_DISCONNECT_POLICY  
    // Call policy to allow custom policy actions in current context (e.g. cancel playback)
    int policyAllowed = Policy_PostAction(queryJ); 
    if (!policyAllowed)
    {
        afb_req_fail(req, "Audio policy violation", "Post sound action not allowed in current context");
        return;
    }
#endif

    //afb_event_push(g_AHLCtx.policyCtx.postEvent,queryJ);

    afb_req_success(req, NULL, "Posted sound action");
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
        if(pEventName == NULL) {
            afb_req_fail(req, "failed", "Invalid event");
			return;
        }
        else if(!strcasecmp(pEventName, AHL_ENDPOINT_PROPERTY_EVENT)) {
			afb_req_subscribe(req, g_AHLCtx.policyCtx.propertyEvent);
            AFB_DEBUG("Client subscribed to endpoint property events");
		}
        else if(!strcasecmp(pEventName, AHL_ENDPOINT_VOLUME_EVENT)) {
			afb_req_subscribe(req, g_AHLCtx.policyCtx.volumeEvent);
            AFB_DEBUG("Client subscribed to endpoint volume events");
		}
        else if(!strcasecmp(pEventName, AHL_POST_ACTION_EVENT)) {
			afb_req_subscribe(req, g_AHLCtx.policyCtx.postActionEvent);
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
        if(pEventName == NULL) {
            afb_req_fail(req, "failed", "Invalid event");
			return;
        }
        else if(!strcasecmp(pEventName, AHL_ENDPOINT_PROPERTY_EVENT)) {
			afb_req_unsubscribe(req, g_AHLCtx.policyCtx.propertyEvent);
            AFB_DEBUG("Client unsubscribed to endpoint property events");
		}
        else if(!strcasecmp(pEventName, AHL_ENDPOINT_VOLUME_EVENT)) {
			afb_req_unsubscribe(req, g_AHLCtx.policyCtx.volumeEvent);
            AFB_DEBUG("Client unsubscribed to endpoint volume events");
		}
        else if(!strcasecmp(pEventName, AHL_POST_ACTION_EVENT)) {
			afb_req_unsubscribe(req, g_AHLCtx.policyCtx.postActionEvent);
            AFB_DEBUG("Client unsubscribed to post event calls events");
		}
        else {
			afb_req_fail(req, "failed", "Invalid event");
			return;
		}
    }

    afb_req_success(req, NULL, "Unsubscribe to events finished");
}

// Since the policy is currently in the same binding, it cannot raise events on its own
// This is a first step toward isolation, when policy is migrated in its own binding it can simply raise AGL events
// This binding will register for these policy events and will execute the code below upon event reception
PUBLIC void audiohlapi_raise_event(json_object * pEventDataJ)
{
    char * pEventName = NULL;

    int err = wrap_json_unpack(pEventDataJ,"{s:s}","event_name", &pEventName);
    if(err)
    {
       AFB_ERROR("Unable to retrieve event name");
       return;
    }

    if(strcasecmp(pEventName, AHL_ENDPOINT_INIT_EVENT)==0) {
        char * pAudioRole = NULL;
        endpointID_t endpointID = AHL_UNDEFINED;
        EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
        int err = wrap_json_unpack(pEventDataJ,"{s:i,s:i,s:s}",
                            "endpoint_id", &endpointID, 
                            "endpoint_type", &endpointType,
                            "audio_role", &pAudioRole);
        if(err)
        {
            AFB_ERROR("Unable to unpack property init event");
            return;
        }

        RoleInfoT * pRole = GetRole(pAudioRole);
        if ( pRole == NULL ){
            AFB_ERROR("Requested audio role does not exist in current configuration -> %s", pAudioRole);
            return;
        }
    
        EndpointInfoT * pEndpointInfo = InitEndpointInfo();
        g_assert_nonnull(pEndpointInfo);
        PolicyCtxJSONToEndpoint(pEventDataJ,pEndpointInfo);

        err = ReplaceEndpointInfoWithRole(endpointID,endpointType,pRole,pEndpointInfo);        
        if(err == AHL_FAIL)
        {
            AFB_ERROR("Can't update EndpointInfo aborting");
            return;
        }

        // Remove event name from object
        // json_object_object_del(pEventDataJ,"event_name");
        //afb_event_push(g_AHLCtx.policyCtx.propertyEvent,pEventDataJ); // Not broadcasted to application at this time
    }
    else if(strcasecmp(pEventName, AHL_ENDPOINT_PROPERTY_EVENT)==0) {
        char * pAudioRole = NULL;
        char * pPropertyName = NULL;
        endpointID_t endpointID = AHL_UNDEFINED;
        EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
        json_object * propValueJ = NULL;
        int err = wrap_json_unpack(pEventDataJ,"{s:i,s:i,s:s,s:o,s:s}",
                            "endpoint_id", &endpointID, 
                            "endpoint_type", &endpointType,
                            "property_name", &pPropertyName,
                            "value",&propValueJ, 
                            "audio_role", &pAudioRole);
        if(err)
        {
            AFB_ERROR("Unable to unpack property event");
            return;
        }
        RoleInfoT * pRole = GetRole(pAudioRole);
        if ( pRole == NULL ){
            AFB_ERROR("Requested audio role does not exist in current configuration -> %s", pAudioRole);
            return;
        }
        EndpointInfoT * pEndpointInfo = GetEndpointInfoWithRole(endpointID,endpointType,pRole);
        // update property value
        if (pEndpointInfo->pPropTable)
        {
            json_type jType = json_object_get_type(propValueJ);
            switch (jType) {
                case json_type_double:
                    Add_Endpoint_Property_Double(pEndpointInfo,pPropertyName,json_object_get_double(propValueJ));
                    break;
                case json_type_int:
                    Add_Endpoint_Property_Int(pEndpointInfo,pPropertyName,json_object_get_int(propValueJ));
                    break;
                case json_type_string:
                    Add_Endpoint_Property_String(pEndpointInfo,pPropertyName,json_object_get_string(propValueJ));
                    break;
                default:
                    AFB_ERROR("Invalid property argument Property value not a valid json object query=%s", json_object_get_string(propValueJ));
                    return ;
            }
        }
        // Remove event name from object
        json_object_object_del(pEventDataJ,"event_name");
        afb_event_push(g_AHLCtx.policyCtx.propertyEvent,pEventDataJ);
    }
    else if(strcasecmp(pEventName, AHL_ENDPOINT_VOLUME_EVENT)==0) {
        char * pAudioRole = NULL;
        endpointID_t endpointID = AHL_UNDEFINED;
        EndpointTypeT endpointType = ENDPOINTTYPE_MAXVALUE;
        int iVolume = 0;
        int err = wrap_json_unpack(pEventDataJ,"{s:i,s:i,s:i,s:s}",
                            "endpoint_id", &endpointID, 
                            "endpoint_type", &endpointType,
                            "value",&iVolume, 
                            "audio_role", &pAudioRole);
        if(err)
        {
            AFB_ERROR("Unable to unpack volume event data");
            return;
        }
        RoleInfoT * pRole = GetRole(pAudioRole);
        if ( pRole == NULL ){
            AFB_ERROR("Requested audio role does not exist in current configuration -> %s", pAudioRole);
            return;
        }
        EndpointInfoT * pEndpointInfo = GetEndpointInfoWithRole(endpointID,endpointType,pRole);
        // update volume value
        pEndpointInfo->iVolume = iVolume;
        // Remove event name from object
        json_object_object_del(pEventDataJ,"event_name");
        afb_event_push(g_AHLCtx.policyCtx.volumeEvent,pEventDataJ);
    }
    else if(strcasecmp(pEventName, AHL_POST_ACTION_EVENT)==0) {
        // Remove event name from object
        json_object_object_del(pEventDataJ,"event_name");
        afb_event_push(g_AHLCtx.policyCtx.postActionEvent,pEventDataJ);
    }
    else if(strcasecmp(pEventName, AHL_STREAM_STATE_EVENT)==0) {
        streamID_t streamID = AHL_UNDEFINED;
        StreamEventT streamEvent = STREAM_EVENT_MAXVALUE;
        int err = wrap_json_unpack(pEventDataJ,"{s:i,s:i}",
                            "stream_id", &streamID, 
                            "state_event", &streamEvent);
        if(err)
        {
            AFB_ERROR("Unable to unpack stream event data");
            return;
        }

        StreamInfoT * pStreamInfo = GetStream(streamID);
        if (pStreamInfo == NULL) {
            AFB_ERROR("Specified stream not currently active stream_id -> %d",streamID);
            return;
        }

        // update streamstate value
        switch (streamEvent) {
            case STREAM_EVENT_START:   
                pStreamInfo->streamState = STREAM_STATE_RUNNING;
                break;
            case STREAM_EVENT_STOP:   
                pStreamInfo->streamState = STREAM_STATE_IDLE;      
                break;
            case STREAM_EVENT_PAUSE: 
                pStreamInfo->streamState = STREAM_STATE_PAUSED;     
                break;
            case STREAM_EVENT_RESUME:       
                pStreamInfo->streamState = STREAM_STATE_RUNNING;    
                break;
            case STREAM_EVENT_MUTED:    
                pStreamInfo->streamMute = STREAM_MUTED;  
                break;
            case STREAM_EVENT_UNMUTED:    
                pStreamInfo->streamMute = STREAM_UNMUTED;
                break;
            default:
                AFB_ERROR("Unknown stream event");
        }
        
        // Remove event name from object
        json_object_object_del(pEventDataJ,"event_name");
        AFB_ERROR("pEventDataJ=%s", json_object_get_string(pEventDataJ));
        afb_event_push(pStreamInfo->streamStateEvent,pEventDataJ);   
    }
    else {        
        AFB_ERROR("Unknown event name");        
    }        
}