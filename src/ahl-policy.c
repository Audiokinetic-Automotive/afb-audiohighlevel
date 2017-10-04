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

// This file provides example of custom, business logic driven policy actions that can affect behavior of the high level
// TODO: Currently only isolated in separate source file. Objective is to make this to at least a shared lib plug-in (shared C context)
// Going a step further would be to implement this within a distinct policy binding (requires to switch to JSON interface)

extern AHLCtxT g_AHLCtx; // TODO: Cannot stay if moved to external module 

static void Add_Endpoint_Property_Numeric( EndpointInfoT * io_pEndpointInfo, char * in_pPropertyName, int in_iPropertyValue)
{
    json_object * propValueJ = json_object_new_int(in_iPropertyValue);
    g_hash_table_insert(io_pEndpointInfo->pPropTable, in_pPropertyName, propValueJ);
}

static void Add_Endpoint_Property_String( EndpointInfoT * io_pEndpointInfo, char * in_pPropertyName, char * in_pPropertyValue)
{
    json_object * propValueJ = json_object_new_string(in_pPropertyValue);
    g_hash_table_insert(io_pEndpointInfo->pPropTable, in_pPropertyName, propValueJ);
}

int Policy_OpenStream(char *pAudioRole, EndpointTypeT endpointType, endpointID_t endpointID)
{
    // TODO: Example rule -> when system is in shutdown or low power mode, no audio stream can be opened (return AHL_POLICY_REJECT)

    return AHL_POLICY_ACCEPT; 
}

int Policy_CloseStream(streamID_t streamID)
{
    // For completeness, unlikely to have any policy rules here

    return AHL_POLICY_ACCEPT; 
}

int  Policy_SetStreamState(streamID_t streamID, StreamStateT streamState )
{
    // If higher priority audio role stream requires audio ducking (and un-ducking) of other streams (e.g. navigation ducks entertainment)
    // TODO: Could potentially provide a fairly generic system using interupt behavior information and audio role priority (implemented and can be customized here)
    // Specific exception can also be

    // Source exclusion. E.g. When a source (e.g tuner) with same audio role as already active stream (e.g. media player) with same endpoint target, 
    // the former source is stopped (i.e. raise streamstate stop event)

    // If source on communication role is active (e.g. handsfree call), activating entertainment sources is prohibited

    // If handsfree or speech recognition (communication role) is started during entertainment playback, mute all entertainment streams (any endpoints except RSE)

    // Startup or Shutdown audio stream mute entertainment (unmut when stream no longer active)

    // Optional: Mute endpoint before activation, unmute afterwards (after a delay?) to avoid noises


    return AHL_POLICY_ACCEPT; 
}

int  Policy_SetStreamMute(streamID_t streamID, StreamMuteT streamMute)
{
    return AHL_POLICY_ACCEPT;
}

int Policy_SetVolume(EndpointTypeT endpointType, endpointID_t endpointID, char *volumeStr)
{
    return AHL_POLICY_ACCEPT; 
}

int Policy_SetProperty(EndpointTypeT endpointType, endpointID_t endpointID, char *propertyName, char *propValueStr)
{
    return AHL_POLICY_ACCEPT;
}

int Policy_PostEvent(char *eventName, char *audioRole, char *mediaName, void *audioContext)
{
    // TODO: Any event with media specified should trigger action on provided rendering services (e.g. Wwise binding, gstreamer file player wrapper, MPDC? simple ALSA player (aplay)?)

    // Example (when the policy is hooked to CAN events). Post audio playback events other than safety during reverse gear engaged declined

    // Example post HMI audio role playback events declined when higher priority streams are active

    return AHL_POLICY_ACCEPT;
}

int Policy_AudioDeviceChange()
{
    // Allow or disallow a new audio endpoint to be used by the system
    // TODO: Policy assigns audio role(s) for device (or default)
    // TODO: Raise events to engage device switching if active stream in audio role assigned to the new endpoint
    
    return AHL_POLICY_ACCEPT; 
}

int Policy_Endpoint_Property_Init(EndpointInfoT * io_EndpointInfo)
{
    // Populate list of supported properties for specified endpoint (use helper functions)
    // Setup initial values for all properties

    // TODO: Switch on different known endpoints to populate different properties

    // Test example
    Add_Endpoint_Property_Numeric(io_EndpointInfo,AHL_PROPERTY_EQ_LOW,3);
    Add_Endpoint_Property_Numeric(io_EndpointInfo,AHL_PROPERTY_EQ_MID,0);
    Add_Endpoint_Property_Numeric(io_EndpointInfo,AHL_PROPERTY_EQ_HIGH,6);
    Add_Endpoint_Property_Numeric(io_EndpointInfo,AHL_PROPERTY_BALANCE,0);
    Add_Endpoint_Property_Numeric(io_EndpointInfo,AHL_PROPERTY_FADE,30);
    Add_Endpoint_Property_String(io_EndpointInfo,"preset_name","flat");
    
    return 0; // No errors
}

int Policy_Init()
{
    // Other policy specific initialization
    return 0; // No errors
}

void Policy_Term()
{
    // Policy termination
}
