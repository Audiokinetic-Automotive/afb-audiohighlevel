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

#include "ahl-policy-utils.h"
#include "wrap-json.h"
#include <json-c/json.h>
#include <glib.h>

void Add_Endpoint_Property_Double( EndpointInfoT * io_pEndpointInfo, char * in_pPropertyName, double in_dPropertyValue)
{
    json_object * propValueJ = json_object_new_double(in_dPropertyValue);
    g_hash_table_insert(io_pEndpointInfo->pPropTable, in_pPropertyName, propValueJ);
}


void Add_Endpoint_Property_Int( EndpointInfoT * io_pEndpointInfo, char * in_pPropertyName, int in_iPropertyValue)
{
    json_object * propValueJ = json_object_new_int(in_iPropertyValue);
    g_hash_table_insert(io_pEndpointInfo->pPropTable, in_pPropertyName, propValueJ);
}

void Add_Endpoint_Property_String( EndpointInfoT * io_pEndpointInfo, char * in_pPropertyName, const char * in_pPropertyValue)
{
    json_object * propValueJ = json_object_new_string(in_pPropertyValue);
    g_hash_table_insert(io_pEndpointInfo->pPropTable, in_pPropertyName, propValueJ);
}

int PolicyEndpointStructToJSON(EndpointInfoT * pEndpointInfo, json_object **ppPolicyEndpointJ)
{
    if(pEndpointInfo == NULL || pEndpointInfo->pPropTable == NULL)
    {
        AFB_ERROR("Invalid PolicyEndpointStructToJSON arguments");
        return AHL_POLICY_UTIL_FAIL;
    } 

    //Create json object for PropTable
    json_object *pPropTableJ = json_object_new_array();
    if(pEndpointInfo->pPropTable) {      
        GHashTableIter iter;
        gpointer key, value;
        g_hash_table_iter_init (&iter, pEndpointInfo->pPropTable);
        while (g_hash_table_iter_next (&iter, &key, &value))
        {
            json_object *pPropertyJ = NULL;
            int error=wrap_json_pack(&pPropertyJ, "{s:s,s:o}",
                        "property_name", (char*)key, 
                        "property_value", value                  
                        );
            if(error)                    
            {
                AFB_ERROR("Unable to pack JSON endpoint, =%s", wrap_json_get_error_string(error));   
                return AHL_POLICY_UTIL_FAIL;             
            }        
            json_object_array_add(pPropTableJ, pPropertyJ);
        }
        AFB_DEBUG("json object query=%s", json_object_get_string(pPropTableJ));
    }

    //Create json object for Endpoint
    int err= wrap_json_pack(ppPolicyEndpointJ, "{s:i,s:i,s:s,s:s,s:s,s:s,s:s,s:i,s:s,s:i,s:i,s:i,s:i,s:i,s:i,s:i,s:o}",
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
        return AHL_POLICY_UTIL_FAIL;
    }    
    AFB_DEBUG("JSON endpoint information=%s", json_object_get_string(*ppPolicyEndpointJ));
    return AHL_POLICY_UTIL_SUCCESS;
}

int PolicyStreamStructToJSON(StreamInfoT * pPolicyStream, json_object **ppPolicyStreamJ)
{
    if(pPolicyStream == NULL)
    {
        AFB_ERROR("Invalid arguments to PolicyStreamStructToJSON");
        return AHL_POLICY_UTIL_FAIL;
    }

    json_object * pEndpointJ = NULL;
    int iRet = PolicyEndpointStructToJSON(pPolicyStream->pEndpointInfo, &pEndpointJ);
    if (iRet) {
        return iRet;
    }

    //Create json object for stream
    int err = wrap_json_pack(ppPolicyStreamJ, "{s:i,s:i,s:i,s:I,s:i,s:s,s:i,s:i,s:o}",
                    "stream_id", pPolicyStream->streamID, 
                    "stream_state", pPolicyStream->streamState,
                    "stream_mute", pPolicyStream->streamMute, 
                    "stream_state_event", &pPolicyStream->streamStateEvent, 
                    "endpoint_sel_mod",  pPolicyStream->endpointSelMode,
                    "role_name", pPolicyStream->pRoleName,
                    "priority", pPolicyStream->iPriority,
                    "interrupt_behavior", pPolicyStream->eInterruptBehavior,
                    "endpoint_info", pEndpointJ
                    );
    if (err) {
        AFB_ERROR("Unable to pack JSON endpoint, =%s", wrap_json_get_error_string(err));
        return AHL_POLICY_UTIL_FAIL;
    }

    AFB_DEBUG("JSON stream information=%s", json_object_get_string(*ppPolicyStreamJ));

    return AHL_POLICY_UTIL_SUCCESS;
}

int PolicyCtxJSONToEndpoint(json_object *pEndpointJ, EndpointInfoT * pEndpointInfo)
{
    if(pEndpointJ == NULL || pEndpointInfo == NULL /*|| pEndpointInfo->pPropTable == NULL */ )
    {
        AFB_ERROR("Invalid arguments for PolicyCtxJSONToEndpoint");
        return AHL_POLICY_UTIL_FAIL;
    }

    //Unpack Endpoint
    json_object *pPropTableJ = NULL;
    int err = wrap_json_unpack(pEndpointJ, "{s:i,s:i,s:s,s:s,s:s,s:s,s:s,s:i,s:s,s:i,s:i,s:i,s:i,s:i,s:i,s:i,s:o}",
                    "endpoint_id", &pEndpointInfo->endpointID, 
                    "endpoint_type", &pEndpointInfo->type,
                    "device_name", &pEndpointInfo->gsDeviceName, 
                    "display_name", &pEndpointInfo->gsDisplayName, 
                    "device_uri",  &pEndpointInfo->gsDeviceURI,
                    "device_domain", &pEndpointInfo->gsDeviceDomain,
                    "audio_role", &pEndpointInfo->pRoleName,
                    "device_uri_type", &pEndpointInfo->deviceURIType,
                    "hal_api_name", &pEndpointInfo->gsHALAPIName,
                    "alsa_cardNum", &pEndpointInfo->alsaInfo.cardNum, 
                    "alsa_deviceNum", &pEndpointInfo->alsaInfo.deviceNum, 
                    "alsa_subDeviceNum", &pEndpointInfo->alsaInfo.subDeviceNum,
                    "format_samplerate", &pEndpointInfo->format.sampleRate,
                    "format_numchannels", &pEndpointInfo->format.numChannels,
                    "format_sampletype",&pEndpointInfo->format.sampleType,
                    "volume", &pEndpointInfo->iVolume,                    
                    "property_table", &pPropTableJ                    
                    );
    if (err) {
        AFB_ERROR("Unable to unpack JSON endpoint, =%s", wrap_json_get_error_string(err));
        return AHL_POLICY_UTIL_FAIL;
    }

    // Unpack prop table
    if(pPropTableJ)
    {
        pEndpointInfo->pPropTable = g_hash_table_new(g_str_hash, g_str_equal);

        int nbProperties = json_object_array_length(pPropTableJ);
        for(int i=0; i<nbProperties; i++)
        {
            json_object * propJ = json_object_array_get_idx(pPropTableJ,i);
            if (propJ) {
                char * pPropertyName = NULL;
                json_object * pPropertyValueJ = NULL;

                int err=wrap_json_unpack(propJ, "{s:s,s:o}",
                                        "property_name", &pPropertyName, 
                                        "property_value", &pPropertyValueJ);
                if (err) {
                    AFB_ERROR("Unable to unpack JSON endpoint, = %s", wrap_json_get_error_string(err));
                    return AHL_POLICY_UTIL_FAIL;
                }        

                // Object type detection for property value (string = state, numeric = property)
                json_type jType = json_object_get_type(pPropertyValueJ);
                switch (jType) {
                    case json_type_double:
                        Add_Endpoint_Property_Double(pEndpointInfo,pPropertyName,json_object_get_double(pPropertyValueJ));
                        break;
                    case json_type_int:
                        Add_Endpoint_Property_Int(pEndpointInfo,pPropertyName,json_object_get_int(pPropertyValueJ));
                        break;
                    case json_type_string:
                        Add_Endpoint_Property_String(pEndpointInfo,pPropertyName,json_object_get_string(pPropertyValueJ));
                        break;
                    default:
                        AFB_ERROR("Invalid property argument Property value not a valid json object query=%s", json_object_get_string(pPropertyValueJ));
                        return AHL_POLICY_UTIL_FAIL;
                }
            }
        }
    }

    return AHL_POLICY_UTIL_SUCCESS;
}

int PolicyCtxJSONToStream(json_object *pStreamJ, StreamInfoT * pPolicyStream)
{
    if(pStreamJ == NULL || pPolicyStream == NULL)
    {
        AFB_ERROR("Invalid arguments for PolicyCtxJSONToStream");
        return AHL_POLICY_UTIL_FAIL;
    }

    //Unpack StreamInfo
    json_object *pEndpointJ = NULL;
    AFB_WARNING("json object query=%s", json_object_get_string(pStreamJ));
    int err=wrap_json_unpack(pStreamJ, "{s:i,s:i,s:i,s:I,s:i,s:s,s:i,s:i,s:o}",
                    "stream_id", &pPolicyStream->streamID, 
                    "stream_state", &pPolicyStream->streamState,
                    "stream_mute", &pPolicyStream->streamMute, 
                    "stream_state_event", &pPolicyStream->streamStateEvent, 
                    "endpoint_sel_mod",  &pPolicyStream->endpointSelMode,
                    "role_name", &pPolicyStream->pRoleName,
                    "priority", &pPolicyStream->iPriority,
                    "interrupt_behavior", &pPolicyStream->eInterruptBehavior,
                    "endpoint_info", &pEndpointJ
                    );

    if (err) {
        AFB_ERROR("Unable to parse JSON stream information=%s", json_object_get_string(pStreamJ));
        return AHL_POLICY_UTIL_FAIL;
    }

    int iRet = PolicyCtxJSONToEndpoint(pEndpointJ,pPolicyStream->pEndpointInfo);
    if (iRet) {
        return iRet;
    }
    return AHL_POLICY_UTIL_SUCCESS;
}
 