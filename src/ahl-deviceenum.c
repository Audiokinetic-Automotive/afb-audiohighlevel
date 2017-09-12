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

static endpointID_t CreateNewSourceID()
{
    endpointID_t newID = g_AHLCtx.nextSourceEndpointID;
    g_AHLCtx.nextSourceEndpointID++;
    return newID;
}

static endpointID_t CreateNewSinkID()
{
    endpointID_t newID = g_AHLCtx.nextSinkEndpointID;
    g_AHLCtx.nextSinkEndpointID++;
    return newID;
}

// Watchout: This function uses strtok and is destructive on the input string (use a copy)
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
    return (int) (strcasecmp(in_pDomainStr,AUDIOHL_DOMAIN_ALSA) == 0);
}

static int IsPulseDomain(const char * in_pDomainStr)
{
    return (int) (strcasecmp(in_pDomainStr,AUDIOHL_DOMAIN_PULSE) == 0);
}

static int IsGStreamerDomain(const char * in_pDomainStr)
{
    return (int) (strcasecmp(in_pDomainStr,AUDIOHL_DOMAIN_GSTREAMER) == 0);
}

static int IsExternalDomain(const char * in_pDomainStr)
{
    return (int) (strcasecmp(in_pDomainStr,AUDIOHL_DOMAIN_EXTERNAL) == 0);
}

static int FillALSAPCMInfo( snd_pcm_t * in_pPcmHandle, EndpointInfoT * out_pEndpointInfo )         
{   
    snd_pcm_type_t pcmType = 0;
    snd_pcm_info_t * pPcmInfo = NULL;
    int iAlsaRet = 0;
    const char * pDeviceName = NULL;
    int retVal = 0;

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
        default:
            out_pEndpointInfo->deviceURIType = DEVICEURITYPE_ALSA_OTHER;
            break;
    }

    iAlsaRet = snd_pcm_info_malloc(&pPcmInfo);
    if (iAlsaRet < 0)
    {
        AFB_WARNING("Error allocating PCM info structure");
        retVal = 1;
        goto End;
    }

    iAlsaRet = snd_pcm_info(in_pPcmHandle,pPcmInfo);
    if (iAlsaRet < 0)
    {
        AFB_WARNING("Error retrieving PCM device info");
        retVal = 1;
        goto End;
    }

    // Populate target device name (for application display)
    pDeviceName = snd_pcm_info_get_name(pPcmInfo);
    if (pDeviceName == NULL)
    {
        AFB_WARNING("No Alsa device name available");
        retVal = 1;
        goto End;
        // Could potentially assign a "default" name and carry on with this device
    }
    strncpy(out_pEndpointInfo->deviceName,pDeviceName,AUDIOHL_MAX_DEVICE_NAME_LENGTH); // Overwritten by HAL name if available

    // get card number
    out_pEndpointInfo->cardNum = snd_pcm_info_get_card(pPcmInfo);
    if ( out_pEndpointInfo->cardNum < 0 )
    {
        AFB_WARNING("No Alsa card number available");
        retVal = 1;
        goto End; 
    }
    
    // get device number
    out_pEndpointInfo->deviceNum = snd_pcm_info_get_device(pPcmInfo);
    if ( out_pEndpointInfo->deviceNum < 0 )
    {
        AFB_WARNING("No Alsa device number available");
        retVal = 1;
        goto End;
    }

    // get sub-device number
    out_pEndpointInfo->subDeviceNum = snd_pcm_info_get_subdevice(pPcmInfo);
    if ( out_pEndpointInfo->subDeviceNum < 0 )
    {
        AFB_WARNING("No Alsa subdevice number available");
        retVal = 1;
        goto End; 
    }

End:
    if(pPcmInfo) {
        snd_pcm_info_free(pPcmInfo);
        pPcmInfo = NULL;
    }

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
        if (iCardNum == io_pEndpointInfo->cardNum) {
            strncpy(io_pEndpointInfo->halAPIName,pAPIName,AUDIOHL_MAX_AUDIOROLE_LENGTH);
            strncpy(io_pEndpointInfo->deviceName,pShortName,AUDIOHL_MAX_DEVICE_NAME_LENGTH);
            found = 1;
            break;
        }
    }
    return !found;
}

static int InitializeEndpointStates( EndpointInfoT * out_pEndpointInfo )
{
    //out_pEndpointInfo = g_array_sized_new(FALSE,TRUE,sizeof)
    // for list of known states
    return 0;
}

// For a given audio role
PUBLIC int EnumerateSources(json_object * in_jSourceArray, int in_iRoleIndex, char * in_pRoleName) {

    int iNumberDevices = json_object_array_length(in_jSourceArray);

    // Parse and validate list of available devices
    for (unsigned int i = 0; i < iNumberDevices; i++)
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
        
        // non ALSA URI are simply passed to application (no validation) at this time 
        // In Non ALSA case devices in config are assumed to be available, if not application can fallback on explicit device selection 
        endpointInfo.cardNum = -1;
        endpointInfo.deviceNum = -1;
        endpointInfo.cardNum = -1;
        strncpy(endpointInfo.deviceName,pDeviceURIPCM,AUDIOHL_MAX_DEVICE_NAME_LENGTH); // Overwritten by HAL name if available

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

            // Retrieve HAL API name
            err = RetrieveAssociatedHALAPIName(&endpointInfo);
            if (err) {
                AFB_WARNING("SetVolume will fail without HAL association ->%s",endpointInfo.deviceURI);
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

        err = InitializeEndpointStates( &endpointInfo );
        if (err) {
            AFB_ERROR("Cannot initialize endpoint states for URI -> %s",fullDeviceURI);
            continue;
        }

        strncpy(endpointInfo.deviceURI,pDeviceURIPCM,AUDIOHL_MAX_DEVICE_URI_LENGTH);
        strncpy(endpointInfo.audioRole,in_pRoleName,AUDIOHL_MAX_AUDIOROLE_LENGTH);
        endpointInfo.endpointID = CreateNewSourceID();
        endpointInfo.type = ENDPOINTTYPE_SOURCE;

        // add to structure to list of available source devices
        GArray * pRoleSourceDeviceArray = g_ptr_array_index( g_AHLCtx.policyCtx.pSourceEndpoints, in_iRoleIndex );
        g_array_append_val(pRoleSourceDeviceArray, endpointInfo);

    } // for all devices

    AFB_DEBUG ("Audio high-level - Enumerate sources done");
    return 0;
}

// For a given audio role
PUBLIC int EnumerateSinks(json_object * in_jSinkArray, int in_iRoleIndex, char * in_pRoleName) {

    int iNumberDevices = json_object_array_length(in_jSinkArray);

    // Parse and validate list of available devices
    for (unsigned int i = 0; i < iNumberDevices; i++)
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

        endpointInfo.cardNum = -1;
        endpointInfo.deviceNum = -1;
        endpointInfo.cardNum = -1;
        strncpy(endpointInfo.deviceName,pDeviceURIPCM,AUDIOHL_MAX_DEVICE_NAME_LENGTH);

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

        err = InitializeEndpointStates( &endpointInfo );
        if (err) {
            AFB_ERROR("Cannot initialize endpoint states for URI -> %s",fullDeviceURI);
            continue;
        }

        strncpy(endpointInfo.deviceURI,pDeviceURIPCM,AUDIOHL_MAX_DEVICE_URI_LENGTH);
        strncpy(endpointInfo.audioRole,in_pRoleName,AUDIOHL_MAX_AUDIOROLE_LENGTH);
        endpointInfo.endpointID = CreateNewSinkID();
        endpointInfo.type = ENDPOINTTYPE_SINK;

        // add to structure to list of available source devices
        GArray * pRoleSinkDeviceArray = g_ptr_array_index( g_AHLCtx.policyCtx.pSinkEndpoints, in_iRoleIndex );
        g_array_append_val(pRoleSinkDeviceArray, endpointInfo);

    } // for all devices

    AFB_DEBUG ("Audio high-level - Enumerate sinks done");
    return 0;
}
