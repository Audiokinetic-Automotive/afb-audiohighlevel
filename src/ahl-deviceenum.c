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
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include <json-c/json.h>
#include "wrap-json.h"

#include "ahl-binding.h"

extern AHLCtxT g_AHLCtx;

// TODO: Hash from endpoint ID information instead
static endpointID_t CreateNewSourceID()
{
    endpointID_t newID = g_AHLCtx.nextSourceEndpointID;
    g_AHLCtx.nextSourceEndpointID++;
    return newID;
}

// TODO: Hash from endpoint ID information instead
static endpointID_t CreateNewSinkID()
{
    endpointID_t newID = g_AHLCtx.nextSinkEndpointID;
    g_AHLCtx.nextSinkEndpointID++;
    return newID;
}

// Watchout: This function uses strtok and is destructive on the input string (use a copy)
// TODO: Perhaps it would be clearer to separate domain and device URI in both API inputs and outputs
static int SeparateDomainFromDeviceURI( char * in_pDeviceURI, char ** out_pDomain, char ** out_pDevice)
{
    *out_pDomain = strtok(in_pDeviceURI, ".");
    if (*out_pDomain == NULL)
    {
        AFB_ERROR("Error tokenizing device URI -> %s",in_pDeviceURI);
        return 1;
    }
    // TODO: Validate domain is known string (e.g. ALSA,Pulse,GStreamer)
    *out_pDevice = strtok(NULL, ".");
    if (*out_pDevice == NULL)
    {
        AFB_ERROR("Error tokenizing device URI -> %s",in_pDeviceURI);
        return 1;
    }
    return 0;
}

static int IsAlsaDomain(const char * in_pDomainStr)
{
    return (int) (strcasecmp(in_pDomainStr,AHL_DOMAIN_ALSA) == 0);
}

static int IsPulseDomain(const char * in_pDomainStr)
{
    return (int) (strcasecmp(in_pDomainStr,AHL_DOMAIN_PULSE) == 0);
}

static int IsGStreamerDomain(const char * in_pDomainStr)
{
    return (int) (strcasecmp(in_pDomainStr,AHL_DOMAIN_GSTREAMER) == 0);
}

static int IsExternalDomain(const char * in_pDomainStr)
{
    return (int) (strcasecmp(in_pDomainStr,AHL_DOMAIN_EXTERNAL) == 0);
}

static int FillALSAPCMInfo( snd_pcm_t * in_pPcmHandle, EndpointInfoT * out_pEndpointInfo )         
{   
    snd_pcm_type_t pcmType = 0;
    snd_pcm_info_t * pPcmInfo = NULL;
    int iAlsaRet = 0;
    const char * pCardName = NULL;
    int retVal = 0;
    snd_ctl_t * ctlHandle = NULL;
	snd_ctl_card_info_t * ctlInfo = NULL;

	snd_pcm_info_alloca(&pPcmInfo);
    snd_ctl_card_info_alloca(&ctlInfo);

    // retrieve PCM type
    pcmType = snd_pcm_type(in_pPcmHandle);
    switch (pcmType) {
        case SND_PCM_TYPE_HW:
            out_pEndpointInfo->deviceURIType = DEVICEURITYPE_ALSA_HW;
            break;
        case SND_PCM_TYPE_DMIX:
            out_pEndpointInfo->deviceURIType = DEVICEURITYPE_ALSA_DMIX;
            break;
        case SND_PCM_TYPE_SOFTVOL:
            out_pEndpointInfo->deviceURIType = DEVICEURITYPE_ALSA_SOFTVOL;
            break;
        case SND_PCM_TYPE_PLUG:
            out_pEndpointInfo->deviceURIType = DEVICEURITYPE_ALSA_PLUG;
            break;
        default:
            out_pEndpointInfo->deviceURIType = DEVICEURITYPE_ALSA_OTHER;
            break;
    }

    iAlsaRet = snd_pcm_info(in_pPcmHandle,pPcmInfo);
    if (iAlsaRet < 0)
    {
        AFB_WARNING("Error retrieving PCM device info");
        return 1;
    }

    // get card number
    out_pEndpointInfo->alsaInfo.cardNum = snd_pcm_info_get_card(pPcmInfo);
    if ( out_pEndpointInfo->alsaInfo.cardNum < 0 )
    {
        AFB_WARNING("No Alsa card number available");
        return 1;
    }
    
    // get device number
    out_pEndpointInfo->alsaInfo.deviceNum = snd_pcm_info_get_device(pPcmInfo);
    if ( out_pEndpointInfo->alsaInfo.deviceNum < 0 )
    {
        AFB_WARNING("No Alsa device number available");
        return 1;
    }

    // get sub-device number
    out_pEndpointInfo->alsaInfo.subDeviceNum = snd_pcm_info_get_subdevice(pPcmInfo);
    if ( out_pEndpointInfo->alsaInfo.subDeviceNum < 0 )
    {
        AFB_WARNING("No Alsa subdevice number available");
        return 1;
    }

    char cardName[32];
	sprintf(cardName, "hw:%d", out_pEndpointInfo->alsaInfo.cardNum);
	iAlsaRet = snd_ctl_open(&ctlHandle, cardName, 0);
    if ( iAlsaRet < 0 )
    {
        AFB_WARNING("Could not open ALSA card control");
        return 1;
    }

	iAlsaRet = snd_ctl_card_info(ctlHandle, ctlInfo);
    if ( iAlsaRet < 0 )
    {
        AFB_WARNING("Could not retrieve ALSA card info");
        snd_ctl_close(ctlHandle);
        return 1;
    }

    // Populate unique target card name 
    pCardName = snd_ctl_card_info_get_id(ctlInfo);
    if (pCardName == NULL)
    {
        AFB_WARNING("No Alsa card name available");
        snd_ctl_close(ctlHandle);
        return 1;
    }
    g_string_assign(out_pEndpointInfo->gsDeviceName,pCardName); 

    snd_ctl_close(ctlHandle);

    return retVal;
}

static int RetrieveAssociatedHALAPIName(EndpointInfoT * io_pEndpointInfo)
{
    json_object *j_response, *j_query = NULL;
    int err;
    err = afb_service_call_sync("alsacore", "hallist", j_query, &j_response);
    if (err) {
        AFB_ERROR("Could not retrieve list of HAL from ALSA core");
        return 1;
    }
    AFB_DEBUG("ALSAcore hallist response=%s", json_object_to_json_string(j_response));

    // Look through returned list for matching card
    int found = 0;
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
            return 1;
        }

        // Retrieve card number (e.g. hw:0)
        int iCardNum = atoi(pDevIDStr+3);
        if (iCardNum == io_pEndpointInfo->alsaInfo.cardNum) {
            g_string_assign(io_pEndpointInfo->gsHALAPIName,pAPIName);
            g_string_assign(io_pEndpointInfo->gsDisplayName,pShortName);
            found = 1;
            break;
        }
    }
    return !found;
}

static void InitEndpointInfo( EndpointInfoT * out_pEndpointInfo )
{
    out_pEndpointInfo->endpointID = AHL_UNDEFINED;
    out_pEndpointInfo->type = ENDPOINTTYPE_MAXVALUE;
    out_pEndpointInfo->gsDeviceName = g_string_new("Unassigned");
    out_pEndpointInfo->gsDisplayName = g_string_new("Unassigned");
    out_pEndpointInfo->gsDeviceURI = g_string_new("Unassigned");
    out_pEndpointInfo->deviceURIType = DEVICEURITYPE_MAXVALUE;
    out_pEndpointInfo->gsAudioRole = g_string_new("Unassigned");
    out_pEndpointInfo->gsHALAPIName = g_string_new("Unassigned");
    out_pEndpointInfo->alsaInfo.cardNum = AHL_UNDEFINED;
    out_pEndpointInfo->alsaInfo.deviceNum = AHL_UNDEFINED;
    out_pEndpointInfo->alsaInfo.subDeviceNum = AHL_UNDEFINED;
    out_pEndpointInfo->format.sampleRate = AHL_UNDEFINED;
    out_pEndpointInfo->format.numChannels = AHL_UNDEFINED;
    out_pEndpointInfo->format.sampleType = AHL_FORMAT_UNKNOWN;
    out_pEndpointInfo->pPropTable = g_hash_table_new(g_str_hash, g_str_equal);
}

static void TermEndpointInfo( EndpointInfoT * out_pEndpointInfo )
{
    g_string_free(out_pEndpointInfo->gsDeviceName,TRUE);
    g_string_free(out_pEndpointInfo->gsDisplayName,TRUE);
    g_string_free(out_pEndpointInfo->gsDeviceURI,TRUE);
    g_string_free(out_pEndpointInfo->gsAudioRole,TRUE);
    g_string_free(out_pEndpointInfo->gsHALAPIName,TRUE);
    // TODO: Free json_object for all property values
    g_hash_table_remove_all(out_pEndpointInfo->pPropTable);
    g_hash_table_destroy(out_pEndpointInfo->pPropTable);
}

void TermEndpoints()
{
    // Sources for each role
    for (int i = 0; i < g_AHLCtx.policyCtx.pSourceEndpoints->len; i++)
    {
        // For each endpoint within the role
        GArray * pRoleDeviceArray = g_ptr_array_index( g_AHLCtx.policyCtx.pSourceEndpoints, i );
        for (int j = 0 ; j < pRoleDeviceArray->len; j++)
        {
            EndpointInfoT * pEndpointInfo = &g_array_index(pRoleDeviceArray,EndpointInfoT,j);
            TermEndpointInfo(pEndpointInfo);
        }
        g_array_free(pRoleDeviceArray,TRUE);
        pRoleDeviceArray = NULL;
    }
    g_ptr_array_free(g_AHLCtx.policyCtx.pSourceEndpoints,TRUE);
    g_AHLCtx.policyCtx.pSourceEndpoints = NULL;

    // Sinks for each role
    for (int i = 0; i < g_AHLCtx.policyCtx.pSinkEndpoints->len; i++)
    {
        // For each endpoint within the role
        GArray * pRoleDeviceArray = g_ptr_array_index( g_AHLCtx.policyCtx.pSinkEndpoints, i );
        for (int j = 0 ; j < pRoleDeviceArray->len; j++)
        {
            EndpointInfoT * pEndpointInfo = &g_array_index(pRoleDeviceArray,EndpointInfoT,j);
            TermEndpointInfo(pEndpointInfo);
        }
        g_array_free(pRoleDeviceArray,TRUE);
        pRoleDeviceArray = NULL;
    }
    g_ptr_array_free(g_AHLCtx.policyCtx.pSinkEndpoints,TRUE);
    g_AHLCtx.policyCtx.pSinkEndpoints = NULL;
}

#define AUDIOHL_MAX_DEVICE_URI_LENGTH 128

// For a given audio role
int EnumerateSources(json_object * in_jSourceArray, int in_iRoleIndex, char * in_pRoleName) {

    int iNumberDevices = json_object_array_length(in_jSourceArray);

    // Parse and validate list of available devices
    for (int i = 0; i < iNumberDevices; i++)
    {
        char fullDeviceURI[AUDIOHL_MAX_DEVICE_URI_LENGTH]; // strtok is destructive
        char * pDeviceURIDomain = NULL;
        char * pFullDeviceURI = NULL;
        char * pDeviceURIPCM = NULL;
        int err = 0;
        EndpointInfoT endpointInfo;

        json_object * jSource = json_object_array_get_idx(in_jSourceArray,i);

        // strip domain name from URI
        pFullDeviceURI = (char *)json_object_get_string(jSource);
        strncpy(fullDeviceURI,pFullDeviceURI,AUDIOHL_MAX_DEVICE_URI_LENGTH);
        err = SeparateDomainFromDeviceURI(fullDeviceURI,&pDeviceURIDomain,&pDeviceURIPCM);
        if (err)
        {
            AFB_WARNING("Invalid device URI string -> %s",fullDeviceURI);
            continue;
        }

        InitEndpointInfo(&endpointInfo);
        
        // non ALSA URI are simply passed to application (no validation) at this time 
        // In Non ALSA case devices in config are assumed to be available, if not application can fallback on explicit device selection 
        g_string_assign(endpointInfo.gsDeviceName,pDeviceURIPCM); 

        if (IsAlsaDomain(pDeviceURIDomain))
        {
            // TODO: Missing support for loose name matching
            // This will require using ALSA hints to get PCM names
            // And would iterate over all available devices matching string (possibly all if no filtering is desired for a certain role)
            
            snd_pcm_t * pPcmHandle = NULL;    

            // Get PCM handle
	        err = snd_pcm_open(&pPcmHandle, pDeviceURIPCM, SND_PCM_STREAM_CAPTURE, 0);
            if (err < 0)
            {
                AFB_NOTICE("Alsa PCM device was not found -> %s", pDeviceURIPCM);
                continue;
            }

            err = FillALSAPCMInfo(pPcmHandle,&endpointInfo);
            if (err) {
                AFB_WARNING("Unable to retrieve PCM information for PCM -> %s",pDeviceURIPCM);
                snd_pcm_close(pPcmHandle);
                continue;
            }

            snd_pcm_close(pPcmHandle);

            // TODO: Consider policy call to determine URI for set volume execution for a particular role (hw?)
            // Retrieve HAL API name
            err = RetrieveAssociatedHALAPIName(&endpointInfo);
            if (err) {
                AFB_WARNING("SetVolume will fail without HAL association ->%s",endpointInfo.gsDeviceURI->str);
                // Choose not to skip anyhow...
            }
        }
        else if (IsPulseDomain(pDeviceURIDomain)) {
            // Pulse domain
            // For now display name is device URI directly, could extrapolated using more heuristics or even usins Pulse API later on
            endpointInfo.deviceURIType = DEVICEURITYPE_PULSE;
         }
        else if (IsGStreamerDomain(pDeviceURIDomain)){
            // GStreamer domain
            // For now display name is device URI directly, could extrapolated using more heuristics or even usins GStreamer API later on
            endpointInfo.deviceURIType = DEVICEURITYPE_GSTREAMER;
        }
        else if (IsExternalDomain(pDeviceURIDomain)){
            // External domain   
            endpointInfo.deviceURIType = DEVICEURITYPE_EXTERNAL;
        }
        else {
            // Unknown domain
            AFB_WARNING("Unknown domain in device URI string -> %s",fullDeviceURI);
            continue;
        }

        g_string_assign(endpointInfo.gsDeviceURI,pDeviceURIPCM);
        g_string_assign(endpointInfo.gsAudioRole,in_pRoleName);
        endpointInfo.endpointID = CreateNewSourceID();
        endpointInfo.type = ENDPOINTTYPE_SOURCE;
        err = Policy_Endpoint_Property_Init(&endpointInfo);
        if (err) {
            AFB_WARNING("Policy endpoint properties initalization failed for endpointid :%d type:%d",endpointInfo.endpointID, endpointInfo.type);
            // Choose not to skip anyhow...
        }

        // add to structure to list of available source devices
        GArray * pRoleSourceDeviceArray = g_ptr_array_index( g_AHLCtx.policyCtx.pSourceEndpoints, in_iRoleIndex );
        g_array_append_val(pRoleSourceDeviceArray, endpointInfo);

    } // for all devices

    AFB_DEBUG ("Audio high-level - Enumerate sources done");
    return 0;
}

// For a given audio role
int EnumerateSinks(json_object * in_jSinkArray, int in_iRoleIndex, char * in_pRoleName) {

    int iNumberDevices = json_object_array_length(in_jSinkArray);

    // Parse and validate list of available devices
    for (int i = 0; i < iNumberDevices; i++)
    {
        char fullDeviceURI[AUDIOHL_MAX_DEVICE_URI_LENGTH]; // strtok is destructive
        char * pDeviceURIDomain = NULL;
        char * pFullDeviceURI = NULL;
        char * pDeviceURIPCM = NULL;
        int err = 0;
        EndpointInfoT endpointInfo;

        json_object * jSink = json_object_array_get_idx(in_jSinkArray,i);

        // strip domain name from URI
        pFullDeviceURI = (char*)json_object_get_string(jSink);
        strncpy(fullDeviceURI,pFullDeviceURI,AUDIOHL_MAX_DEVICE_URI_LENGTH);
        err = SeparateDomainFromDeviceURI(fullDeviceURI,&pDeviceURIDomain,&pDeviceURIPCM);
        if (err)
        {
            AFB_WARNING("Invalid device URI string -> %s",fullDeviceURI);
            continue;
        }
        
        // non ALSA URI are simply passed to application (no validation) at this time 
        // In Non ALSA case devices in config are assumed to be available, if not application can fallback on explicit device selection 

        InitEndpointInfo(&endpointInfo);

        endpointInfo.alsaInfo.cardNum = -1;
        endpointInfo.alsaInfo.deviceNum = -1;
        endpointInfo.alsaInfo.cardNum = -1;
        g_string_assign(endpointInfo.gsDeviceName,pDeviceURIPCM);

        if (IsAlsaDomain(pDeviceURIDomain))
        {
            // TODO: Missing support for loose name matching
            // This will require using ALSA hints to get PCM names
            // And would iterate over all available devices matching string (possibly all if no filtering is desired for a certain role)

            snd_pcm_t * pPcmHandle = NULL;

            // get PCM handle
	        err = snd_pcm_open(&pPcmHandle, pDeviceURIPCM, SND_PCM_STREAM_PLAYBACK, 0);
            if (err < 0)
            {
                AFB_NOTICE("Alsa PCM device was not found -> %s", pDeviceURIPCM);
                continue;
            }

            err = FillALSAPCMInfo(pPcmHandle,&endpointInfo);
            if (err) {
                AFB_WARNING("Unable to retrieve PCM information for PCM -> %s",pDeviceURIPCM);
                snd_pcm_close(pPcmHandle);
                continue;
            }

            snd_pcm_close(pPcmHandle);

            // TODO: Consider policy call to determine URI for set volume execution for a particular role (hw?)
            // Retrieve HAL API name
            err = RetrieveAssociatedHALAPIName(&endpointInfo);
            if (err) {
                //AFB_WARNING("SetVolume w fail without HAL association ->%s",endpointInfo.deviceURI);
                continue; 
            }
        }
        else if (IsPulseDomain(pDeviceURIDomain)) {
            // Pulse domain
            // For now display name is device URI directly, could extrapolated using more heuristics or even usins Pulse API later on
            endpointInfo.deviceURIType = DEVICEURITYPE_PULSE;

         }
        else if (IsGStreamerDomain(pDeviceURIDomain)){
            // GStreamer domain
            // For now display name is device URI directly, could extrapolated using more heuristics or even usins GStreamer API later on
            endpointInfo.deviceURIType = DEVICEURITYPE_GSTREAMER;
        }
        else if (IsExternalDomain(pDeviceURIDomain)){
            // External domain
            
            endpointInfo.deviceURIType = DEVICEURITYPE_EXTERNAL;
        }
        else {
            // Unknown domain
            AFB_WARNING("Unknown domain in device URI string -> %s",fullDeviceURI);
            continue;
        }

        g_string_assign(endpointInfo.gsDeviceURI,pDeviceURIPCM);
        g_string_assign(endpointInfo.gsAudioRole,in_pRoleName);
        endpointInfo.endpointID = CreateNewSinkID();
        endpointInfo.type = ENDPOINTTYPE_SINK;
        err = Policy_Endpoint_Property_Init(&endpointInfo);
        if (err) {
            AFB_WARNING("Policy endpoint properties initalization failed for endpointid :%d type:%d",endpointInfo.endpointID, endpointInfo.type);
            // Choose not to skip anyhow...
        }

        // add to structure to list of available source devices
        GArray * pRoleSinkDeviceArray = g_ptr_array_index( g_AHLCtx.policyCtx.pSinkEndpoints, in_iRoleIndex );
        g_array_append_val(pRoleSinkDeviceArray, endpointInfo);

    } // for all devices

    AFB_DEBUG ("Audio high-level - Enumerate sinks done");
    return 0;
}
