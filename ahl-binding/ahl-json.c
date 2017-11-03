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

#define AFB_BINDING_VERSION 2
#include <afb/afb-binding.h>
#include "wrap-json.h"
#include <json-c/json.h>
#include <glib.h>
#include "ahl-binding.h"

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

static int EndpointPropTableToJSON(GHashTable * pPropTable, json_object **ppProptableJ)
{
    if(pPropTable == NULL)
    {
        AFB_ERROR("Invalid EndpointPropTableToJSON arguments");
        return AHL_FAIL;
    } 

    // Create json object for PropTable
    *ppProptableJ = json_object_new_array();     
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init (&iter, pPropTable);
    while (g_hash_table_iter_next (&iter, &key, &value))
    {
        if ( key != NULL && value != NULL) {
            json_object *pPropertyJ = NULL;
            json_object_get(value);
            int err = wrap_json_pack(&pPropertyJ, "{s:s,s:o}",
                        "property_name", (char*)key, 
                        "property_value", value                  
                        );
            if(err)                    
            {
                AFB_ERROR("Unable to pack JSON endpoint, =%s", wrap_json_get_error_string(err));   
                return AHL_FAIL;             
            }        
            json_object_array_add(*ppProptableJ, pPropertyJ);
        }
    }

    return AHL_SUCCESS;
}

int EndpointInfoToJSON(EndpointInfoT * pEndpointInfo, json_object **ppEndpointInfoJ)
{
    if(pEndpointInfo == NULL || pEndpointInfo->pPropTable == NULL)
    {
        AFB_ERROR("Invalid EndpointInfoToJSON arguments");
        return AHL_FAIL;
    } 

    json_object * pPropTableJ = NULL;
    int err = EndpointPropTableToJSON(pEndpointInfo->pPropTable,&pPropTableJ);
    if (err) {
        return AHL_FAIL;
    }

    // Create json object for EndpointInfo
    err = wrap_json_pack(ppEndpointInfoJ, "{s:i,s:i,s:s,s:s,s:s,s:s,s:s,s:i,s:s,s:i,s:i,s:i,s:i,s:i,s:i,s:i,s:o}",
                    "endpoint_id", pEndpointInfo->endpointID, 
                    "endpoint_type", pEndpointInfo->type,
                    "device_name", pEndpointInfo->gsDeviceName, 
                    "display_name", pEndpointInfo->gsDisplayName, 
                    "device_uri",  pEndpointInfo->gsDeviceURI,
                    "device_domain", pEndpointInfo->gsDeviceDomain,
                    "audio_role",pEndpointInfo->pRoleName,
                    "device_uri_type", pEndpointInfo->deviceURIType,
                    "hal_api_name", pEndpointInfo->gsHALAPIName,
                    "alsa_cardNum", pEndpointInfo->alsaInfo.cardNum, 
                    "alsa_deviceNum", pEndpointInfo->alsaInfo.deviceNum, 
                    "alsa_subDeviceNum", pEndpointInfo->alsaInfo.subDeviceNum,
                    "format_samplerate", pEndpointInfo->format.sampleRate,
                    "format_numchannels", pEndpointInfo->format.numChannels,
                    "format_sampletype",pEndpointInfo->format.sampleType,
                    "volume", pEndpointInfo->iVolume,
                    "property_table", pPropTableJ
                    );
    if (err) {
        AFB_ERROR("Unable to pack JSON endpoint, =%s", wrap_json_get_error_string(err));
        return AHL_FAIL;
    }    

    return AHL_SUCCESS;
}

int StreamInfoToJSON(StreamInfoT * pStreamInfo, json_object **ppStreamInfoJ)
{
    if(pStreamInfo == NULL)
    {
        AFB_ERROR("Invalid arguments to StreamInfoToJSON");
        return AHL_FAIL;
    }

    json_object * pEndpointInfoJ = NULL;
    int err = EndpointInfoToJSON(pStreamInfo->pEndpointInfo, &pEndpointInfoJ);
    if (err) {
        return AHL_FAIL;
    }

    // Create json object for stream
    err = wrap_json_pack(ppStreamInfoJ, "{s:i,s:i,s:i,s:I,s:i,s:s,s:i,s:i,s:o}",
                    "stream_id", pStreamInfo->streamID, 
                    "stream_state", pStreamInfo->streamState,
                    "stream_mute", pStreamInfo->streamMute, 
                    "stream_state_event", &pStreamInfo->streamStateEvent, 
                    "endpoint_sel_mod",  pStreamInfo->endpointSelMode,
                    "role_name", pStreamInfo->pRoleName,
                    "priority", pStreamInfo->iPriority,
                    "interrupt_behavior", pStreamInfo->eInterruptBehavior,
                    "endpoint_info", pEndpointInfoJ
                    );
    if (err) {
        AFB_ERROR("Unable to pack JSON endpoint, =%s", wrap_json_get_error_string(err));
        return AHL_FAIL;
    }

    return AHL_SUCCESS;
}

static int UpdatePropertyList(GHashTable * pPropTable, json_object * pPropTableJ) {
    if (pPropTable == NULL || pPropTableJ == NULL) {
        AFB_ERROR("Invalid arguments to UpdatePropertyList");
        return AHL_FAIL;
    }
    // Unpack prop table
    int nbProperties = json_object_array_length(pPropTableJ);
    for(int i = 0; i < nbProperties; i++)
    {
        json_object * propJ = json_object_array_get_idx(pPropTableJ,i);
        if (propJ) {
            char * pPropertyName = NULL;
            json_object * pPropertyValueJ = NULL;
            int err = wrap_json_unpack(propJ, "{s:s,s:o}",
                                    "property_name", &pPropertyName, 
                                    "property_value", &pPropertyValueJ);
            if (err) {
                AFB_ERROR("Unable to unpack JSON property, = %s", wrap_json_get_error_string(err));
                return AHL_FAIL;
            }        

            // Object type detection for property value (string = state, numeric = property)
            json_type jType = json_object_get_type(pPropertyValueJ);
            switch (jType) {
                case json_type_double:
                    g_hash_table_insert(pPropTable, pPropertyName, json_object_new_double(json_object_get_double(pPropertyValueJ)));
                    break;
                case json_type_int:
                    g_hash_table_insert(pPropTable, pPropertyName, json_object_new_int(json_object_get_int(pPropertyValueJ)));
                    break;
                case json_type_string:
                    g_hash_table_insert(pPropTable, pPropertyName, json_object_new_string(json_object_get_string(pPropertyValueJ)));
                    break;
                default:
                    AFB_ERROR("Invalid property argument Property value not a valid json object query=%s", json_object_get_string(pPropertyValueJ));
                    return AHL_FAIL;
            }
        }
    }

    return AHL_SUCCESS;
}

int UpdateEndpointInfo(EndpointInfoT * pEndpoint, json_object * pEndpointInfoJ) {

    if(pEndpoint == NULL || pEndpointInfoJ == NULL)
    {
        AFB_ERROR("Invalid arguments to UpdateEndpointInfo");
        return AHL_FAIL;
    }

     // Push information to endpoint info struct
    json_object * pPropTableJ = NULL;
    char * pDisplayName = NULL;
    char * pHALName = NULL;
    int err = wrap_json_unpack(pEndpointInfoJ,"{s:i,s:s,s:s,s:o}",
                        "init_volume",&pEndpoint->iVolume,
                        "display_name",&pDisplayName,
                        "hal_name", &pHALName,
                        "property_table",&pPropTableJ);
    if (err) {
        AFB_ERROR("Unable to create Endpoint Json object error:%s ",wrap_json_get_error_string(err));
        return AHL_FAIL;
    }
    g_strlcpy(pEndpoint->gsDisplayName,pDisplayName,AHL_STR_MAX_LENGTH);
    g_strlcpy(pEndpoint->gsHALAPIName,pHALName,AHL_STR_MAX_LENGTH);

    if (pEndpoint->pPropTable && pPropTableJ) {
        err = UpdatePropertyList(pEndpoint->pPropTable,pPropTableJ);
        if (err) {
            AFB_ERROR("Unable to update property table Json object error:%s ",wrap_json_get_error_string(err));
            return AHL_FAIL;
        }
    }

    return AHL_SUCCESS;
}

static void AudioFormatStructToJSON(json_object **audioFormatJ, AudioFormatT * pAudioFormat)
{
    wrap_json_pack(audioFormatJ, "{s:i,s:i,s:i}",
                    "sample_rate", pAudioFormat->sampleRate, 
                    "num_channels", pAudioFormat->numChannels, 
                    "sample_type", pAudioFormat->sampleType);
}

// Package only information that can useful to application clients when selecting endpoint
void JSONPublicPackageEndpoint(EndpointInfoT * pEndpointInfo,json_object **endpointInfoJ)
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

    json_object *pPropTableJ = NULL;
    EndpointPropTableToJSON(pEndpointInfo->pPropTable,&pPropTableJ);
    json_object_object_add(*endpointInfoJ,"property_table",pPropTableJ);
}
 
// Package only information that can useful to application clients when opening a stream
void JSONPublicPackageStream(StreamInfoT * pStreamInfo,json_object **streamInfoJ)
{
    json_object *endpointInfoJ = NULL;
    JSONPublicPackageEndpoint(pStreamInfo->pEndpointInfo,&endpointInfoJ);
    wrap_json_pack(streamInfoJ, "{s:i,s:s,s:s,s:s}", 
        "stream_id", pStreamInfo->streamID,
        "state", StreamStateEnumToStr(pStreamInfo->streamState),
        "mute", StreamMuteEnumToStr(pStreamInfo->streamMute),   
        "device_uri",pStreamInfo->pEndpointInfo->gsDeviceURI); // Need to open a stream to have access to the device URI
    json_object_object_add(*streamInfoJ,"endpoint_info",endpointInfoJ);
}