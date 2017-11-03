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

#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
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
static int SeparateDomainFromDeviceURI( char * in_pDeviceURI, char ** out_pDomain, char ** out_pDevice)
{
    *out_pDomain = strtok(in_pDeviceURI, ".");
    if (*out_pDomain == NULL)
    {
        AFB_ERROR("Error tokenizing device URI -> %s",in_pDeviceURI);
        return AHL_FAIL;
    }
    // TODO: Validate domain is known string (e.g. ALSA,Pulse,GStreamer)
    *out_pDevice = strtok(NULL, ".");
    if (*out_pDevice == NULL)
    {
        AFB_ERROR("Error tokenizing device URI -> %s",in_pDeviceURI);
        return AHL_FAIL;
    }
    return AHL_SUCCESS;
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
    g_assert_nonnull(in_pPcmHandle);
    g_assert_nonnull(out_pEndpointInfo);
    snd_pcm_type_t pcmType = 0;
    snd_pcm_info_t * pPcmInfo = NULL;
    int iAlsaRet = 0;
    const char * pCardName = NULL;
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
        return AHL_FAIL;
    }

    // get card number
    out_pEndpointInfo->alsaInfo.cardNum = snd_pcm_info_get_card(pPcmInfo);
    if ( out_pEndpointInfo->alsaInfo.cardNum < 0 )
    {
        AFB_WARNING("No Alsa card number available");
        return AHL_FAIL;
    }
    
    // get device number
    out_pEndpointInfo->alsaInfo.deviceNum = snd_pcm_info_get_device(pPcmInfo);
    if ( out_pEndpointInfo->alsaInfo.deviceNum < 0 )
    {
        AFB_WARNING("No Alsa device number available");
        return AHL_FAIL;
    }

    // get sub-device number
    out_pEndpointInfo->alsaInfo.subDeviceNum = snd_pcm_info_get_subdevice(pPcmInfo);
    if ( out_pEndpointInfo->alsaInfo.subDeviceNum < 0 )
    {
        AFB_WARNING("No Alsa subdevice number available");
        return AHL_FAIL;
    }

    char cardName[32];
	sprintf(cardName, "hw:%d", out_pEndpointInfo->alsaInfo.cardNum);
	iAlsaRet = snd_ctl_open(&ctlHandle, cardName, 0);
    if ( iAlsaRet < 0 )
    {
        AFB_WARNING("Could not open ALSA card control");
        return AHL_FAIL;
    }

	iAlsaRet = snd_ctl_card_info(ctlHandle, ctlInfo);
    if ( iAlsaRet < 0 )
    {
        AFB_WARNING("Could not retrieve ALSA card info");
        snd_ctl_close(ctlHandle);
        return AHL_FAIL;
    }

    // Populate unique target card name 
    pCardName = snd_ctl_card_info_get_id(ctlInfo);
    if (pCardName == NULL)
    {
        AFB_WARNING("No Alsa card name available");
        snd_ctl_close(ctlHandle);
        return AHL_FAIL;
    }
    g_strlcpy(out_pEndpointInfo->gsDeviceName,pCardName,AHL_STR_MAX_LENGTH); 

    snd_ctl_close(ctlHandle);

    return AHL_SUCCESS;
}

EndpointInfoT * InitEndpointInfo()
{
    EndpointInfoT * pEndpointInfo = (EndpointInfoT*) malloc(sizeof(EndpointInfoT));
    memset(pEndpointInfo,0,sizeof(EndpointInfoT));
    pEndpointInfo->endpointID = AHL_UNDEFINED;
    pEndpointInfo->type = ENDPOINTTYPE_MAXVALUE;
    pEndpointInfo->deviceURIType = DEVICEURITYPE_MAXVALUE;
    pEndpointInfo->alsaInfo.cardNum = AHL_UNDEFINED;
    pEndpointInfo->alsaInfo.deviceNum = AHL_UNDEFINED;
    pEndpointInfo->alsaInfo.subDeviceNum = AHL_UNDEFINED;
    pEndpointInfo->format.sampleRate = AHL_UNDEFINED;
    pEndpointInfo->format.numChannels = AHL_UNDEFINED;
    pEndpointInfo->format.sampleType = AHL_FORMAT_UNKNOWN;
    // Assigned by device enumeration
    pEndpointInfo->gsDeviceName = malloc(AHL_STR_MAX_LENGTH*sizeof(char));
    memset(pEndpointInfo->gsDeviceName,0,AHL_STR_MAX_LENGTH*sizeof(char));
    pEndpointInfo->gsDeviceDomain = malloc(AHL_STR_MAX_LENGTH*sizeof(char));
    memset(pEndpointInfo->gsDeviceDomain,0,AHL_STR_MAX_LENGTH*sizeof(char));
    pEndpointInfo->pRoleName = malloc(AHL_STR_MAX_LENGTH*sizeof(char));
    memset(pEndpointInfo->pRoleName,0,AHL_STR_MAX_LENGTH*sizeof(char));
    pEndpointInfo->gsDeviceURI = malloc(AHL_STR_MAX_LENGTH*sizeof(char));
    memset(pEndpointInfo->gsDeviceURI,0,AHL_STR_MAX_LENGTH*sizeof(char));
    // Assigned by policy initialization
    pEndpointInfo->gsDisplayName = malloc(AHL_STR_MAX_LENGTH*sizeof(char));
    memset(pEndpointInfo->gsDisplayName,0,AHL_STR_MAX_LENGTH*sizeof(char));
    pEndpointInfo->gsHALAPIName = malloc(AHL_STR_MAX_LENGTH*sizeof(char));
    memset(pEndpointInfo->gsHALAPIName,0,AHL_STR_MAX_LENGTH*sizeof(char));
    pEndpointInfo->pPropTable = g_hash_table_new(g_str_hash, g_str_equal);
    return pEndpointInfo;
}

void TermEndpointInfo( EndpointInfoT * out_pEndpointInfo )
{
    #define SAFE_FREE(__ptr__) if(__ptr__) g_free(__ptr__); __ptr__ = NULL;
    SAFE_FREE(out_pEndpointInfo->gsDeviceName);  
    SAFE_FREE(out_pEndpointInfo->gsDeviceDomain);
    SAFE_FREE(out_pEndpointInfo->pRoleName);
    SAFE_FREE(out_pEndpointInfo->gsDeviceURI);
    SAFE_FREE(out_pEndpointInfo->gsHALAPIName);
    SAFE_FREE(out_pEndpointInfo->gsDisplayName);

    if (out_pEndpointInfo->pPropTable) {
        // Free json_object for all property values
        GHashTableIter iter;
        gpointer key, value;
        g_hash_table_iter_init (&iter, out_pEndpointInfo->pPropTable);
        while (g_hash_table_iter_next (&iter, &key, &value))
        {
            if (value)
                json_object_put(value);
        }
        g_hash_table_remove_all(out_pEndpointInfo->pPropTable);
        g_hash_table_destroy(out_pEndpointInfo->pPropTable);
        out_pEndpointInfo->pPropTable = NULL;
    }
    // GLib automatically frees item when removed from the array
}

// For a given audio role
int EnumerateDevices(json_object * in_jDeviceArray, char * in_pAudioRole, EndpointTypeT in_deviceType, GPtrArray * out_pEndpointArray) {

    g_assert_nonnull(in_jDeviceArray);
    int iNumberDevices = json_object_array_length(in_jDeviceArray);

    // Parse and validate list of available devices
    for (int i = 0; i < iNumberDevices; i++)
    {
        char * pDeviceURIDomain = NULL;
        char * pFullDeviceURI = NULL;
        char * pDeviceURIPCM = NULL;
        int err = AHL_SUCCESS;

        json_object * jDevice = json_object_array_get_idx(in_jDeviceArray,i);
        if (jDevice == NULL) {
            AFB_WARNING("Invalid device array -> %s",json_object_to_json_string(in_jDeviceArray));
            continue;
        }
        // strip domain name from URI
        pFullDeviceURI = (char *)json_object_get_string(jDevice);
        char * pFullDeviceURICopy = g_strdup(pFullDeviceURI); // strtok is destructive
        err = SeparateDomainFromDeviceURI(pFullDeviceURICopy,&pDeviceURIDomain,&pDeviceURIPCM);
        if (err)
        {
            AFB_WARNING("Invalid device URI string -> %s",pFullDeviceURICopy);
            continue;
        }

        EndpointInfoT * pEndpointInfo = InitEndpointInfo();
        g_assert_nonnull(pEndpointInfo);
        
        // non ALSA URI are simply passed to application (no validation) at this time 
        // In Non ALSA case devices in config are assumed to be available, if not application can fallback on explicit device selection 
        g_strlcpy(pEndpointInfo->gsDeviceName,pDeviceURIPCM,AHL_STR_MAX_LENGTH); 
        g_strlcpy(pEndpointInfo->gsDeviceDomain,pDeviceURIDomain,AHL_STR_MAX_LENGTH);
        g_strlcpy(pEndpointInfo->gsDeviceURI,pDeviceURIPCM,AHL_STR_MAX_LENGTH);
        g_strlcpy(pEndpointInfo->pRoleName ,in_pAudioRole,AHL_STR_MAX_LENGTH);
        
        g_free(pFullDeviceURICopy);
        pFullDeviceURICopy = NULL;
        pDeviceURIDomain = NULL; //Derived from above mem
        pDeviceURIPCM = NULL; //Derived from above mem

        if (IsAlsaDomain(pEndpointInfo->gsDeviceDomain))
        {
            // TODO: Missing support for loose name matching
            // This will require using ALSA hints to get PCM names
            // And would iterate over all available devices matching string (possibly all if no filtering is desired for a certain role)
            
            // Get PCM handle
            snd_pcm_t * pPcmHandle = NULL;  
            snd_pcm_stream_t streamType = in_deviceType == ENDPOINTTYPE_SOURCE ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK;
	        err = snd_pcm_open(&pPcmHandle, pEndpointInfo->gsDeviceURI, streamType, 0);
            if (err < 0)
            {
                AFB_NOTICE("Alsa PCM device was not found -> %s", pEndpointInfo->gsDeviceURI);
                continue;
            }

            err = FillALSAPCMInfo(pPcmHandle,pEndpointInfo);
            if (err) {
                AFB_WARNING("Unable to retrieve PCM information for PCM -> %s",pEndpointInfo->gsDeviceURI);
                snd_pcm_close(pPcmHandle);
                continue;
            }

            snd_pcm_close(pPcmHandle);
        }
        else if (IsPulseDomain(pEndpointInfo->gsDeviceDomain)) {
            // Pulse domain
            // For now display name is device URI directly, could extrapolated using more heuristics or even usins Pulse API later on
            pEndpointInfo->deviceURIType = DEVICEURITYPE_NOT_ALSA;
         }
        else if (IsGStreamerDomain(pEndpointInfo->gsDeviceDomain)){
            // GStreamer domain
            // For now display name is device URI directly, could extrapolated using more heuristics or even usins GStreamer API later on
            pEndpointInfo->deviceURIType = DEVICEURITYPE_NOT_ALSA;
        }
        else if (IsExternalDomain(pEndpointInfo->gsDeviceDomain)){
            // External domain   
            pEndpointInfo->deviceURIType = DEVICEURITYPE_NOT_ALSA;
        }
        else {
            // Unknown domain
            AFB_WARNING("Unknown domain in device URI string -> %s",pFullDeviceURI);
            continue;
        }

        pEndpointInfo->endpointID = in_deviceType == ENDPOINTTYPE_SOURCE ? CreateNewSourceID() : CreateNewSinkID();
        pEndpointInfo->type = in_deviceType;

        // add to structure to list of available devices
        g_ptr_array_add(out_pEndpointArray, pEndpointInfo);

    } // for all devices

    AFB_DEBUG ("Audio high-level - Enumerate devices done");
    return AHL_SUCCESS;
}