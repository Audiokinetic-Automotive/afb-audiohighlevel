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
// TODO: Currently only isolated in separate source file. Objective is to make this to at least a shared lib plug-in
// Going a step further would be to implement this within afb-controler policy plug-in (would require bi-directional access to HLB context)

extern AHLCtxT g_AHLCtx; // TODO: Cannot stay if moved to external module 

// Attribute of high-level binding (parsed), policy enforced
typedef enum InterruptedBehavior {
  INTERRUPTEDBEHAVIOR_CONTINUE = 0, // Continue to play when interrupted (e.g. media will be ducked)
  INTERRUPTEDBEHAVIOR_CANCEL,       // Abort playback when interrupted (e.g. non-important HMI feedback that does not make sense later)
  INTERRUPTEDBEHAVIOR_PAUSE,        // Pause source when interrupted, to be resumed afterwards (e.g. non-temporal guidance)
  INTERRUPTEDBEHAVIOR_MAXVALUE, // Enum count, keep at the end
} InterruptedBehaviorT;

PUBLIC int Policy_OpenStream(char *pAudioRole, EndpointTypeT endpointType, endpointID_t endpointID)
{
    return 1; // Policy allowed
}

PUBLIC int Policy_SetVolume(EndpointTypeT endpointType, endpointID_t endpointID, char *volumeStr, int rampTimeMS)
{
    return 1; // Policy allowed
}

PUBLIC int Policy_SetProperty(EndpointTypeT endpointType, endpointID_t endpointID, char *propertyName, char *propValueStr, int rampTimeMS)
{
    return 1; // Policy allowed
}


PUBLIC int Policy_SetState(EndpointTypeT endpointType, endpointID_t endpointID, char *pStateName, char *pStateValue)
{

    //Active rule check
 
    //Ducking rule settings

    return AUDIOHL_POLICY_ACCEPT;    
}

PUBLIC int Policy_PostSoundEvent(char *eventName, char *audioRole, char *mediaName, void *audioContext)
{
    return 1; // Policy allowed
}

PUBLIC int Policy_AudioDeviceChange()
{
    // Allow or disallow a new audio device
    // Raise events to engage device switching
    // Policy can also assign audio role(s) for device
    return 1; // Policy allowed
}
