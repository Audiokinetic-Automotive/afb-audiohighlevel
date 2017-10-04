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
#include <json-c/json.h>
#include "wrap-json.h"

#include "ahl-binding.h"

extern AHLCtxT g_AHLCtx;

int ParseHLBConfig() {
    char * versionStr = NULL;
    json_object * jAudioRoles = NULL;
    json_object * jHALList = NULL;
    
    // TODO: This should be retrieve from binding startup arguments
    char configfile_path[256];
    sprintf(configfile_path, "%s/opt/config/ahl-config.json", getenv("HOME")); 
    AFB_INFO("High-level config file -> %s\n", configfile_path);

    // Open configuration file
    json_object *config_JFile=json_object_from_file(configfile_path);
    if(config_JFile == NULL)
    {
        AFB_ERROR("Error: Can't open configuration file -> %s",configfile_path);
        return 1;
    }

    int err = wrap_json_unpack(config_JFile, "{s:s,s:o,s:o}", "version", &versionStr,"audio_roles",&jAudioRoles,"hal_list",&jHALList);
    if (err) {
        AFB_ERROR("Invalid configuration file -> %s", configfile_path);
        return 1;
    }

    int iHALListLength = json_object_array_length(jHALList);
    int iNumberOfRoles = json_object_array_length(jAudioRoles);
    g_AHLCtx.policyCtx.iNumberRoles = iNumberOfRoles;

    // Dynamically allocated based on number or roles found
    g_AHLCtx.pHALList = g_array_sized_new(FALSE, TRUE, sizeof(GString), iHALListLength);
    g_AHLCtx.policyCtx.pRolePriority = g_hash_table_new(g_str_hash, g_str_equal);
    g_AHLCtx.policyCtx.pAudioRoles = g_array_sized_new(FALSE, TRUE, sizeof(GString), iNumberOfRoles);
    g_AHLCtx.policyCtx.pInterruptBehavior = g_array_sized_new(FALSE, TRUE, sizeof(int), iNumberOfRoles);
    g_AHLCtx.policyCtx.pSourceEndpoints = g_ptr_array_sized_new(iNumberOfRoles);
    g_AHLCtx.policyCtx.pSinkEndpoints = g_ptr_array_sized_new(iNumberOfRoles);
    g_AHLCtx.policyCtx.pEventList = g_ptr_array_sized_new(iNumberOfRoles);
    g_AHLCtx.policyCtx.iNumberRoles = iNumberOfRoles;
        
    for (int i = 0; i < iHALListLength; i++)
    {
        char * pHAL = NULL;
        json_object * jHAL = json_object_array_get_idx(jHALList,i);
        pHAL = (char *)json_object_get_string(jHAL);
        GString * gHALName = g_string_new( pHAL );
        g_array_append_val( g_AHLCtx.pHALList, *gHALName );

        // Set dependency on HAL
        err = afb_daemon_require_api_v2(pHAL,1) ;
        if( err != 0 )
        {
            AFB_ERROR("Audio high level API could not set dependenvy on API: %s",pHAL);
            return 1;
        }
    }

    for (int i = 0; i < iNumberOfRoles; i++)
    {
        int priority = 0;
        json_object * jAudioRole = json_object_array_get_idx(jAudioRoles,i);
        json_object * jOutputDevices = NULL;
        json_object * jInputDevices = NULL;
        json_object * jEvents = NULL;
        char * pRoleName = NULL;
        InterruptedBehaviorT interupBehavior = AHL_INTERRUPTEDBEHAVIOR_CONTINUE; //Default

        int iNumOutDevices = 0;
        int iNumInDevices = 0;
        int iNumEvents = 0;

        err = wrap_json_unpack(jAudioRole, "{s:s,s:i,s?o,s?o,s?o,s?i}", 
                                    "name", &pRoleName,
                                    "priority",&priority,
                                    "output",&jOutputDevices,
                                    "input",&jInputDevices,
                                    "events",&jEvents,
                                    "interupt_behavior",&interupBehavior);
        if (err) {
            AFB_ERROR("Invalid audio role configuration : %s", json_object_to_json_string(jAudioRole));
            return 1;
        }
        
        if (jOutputDevices)
            iNumOutDevices = json_object_array_length(jOutputDevices);
        if (jInputDevices)
            iNumInDevices = json_object_array_length(jInputDevices);
        if (jEvents)
            iNumEvents = json_object_array_length(jEvents);

        GString * gRoleName = g_string_new( pRoleName );
        g_array_append_val( g_AHLCtx.policyCtx.pAudioRoles, *gRoleName );
        g_hash_table_insert(g_AHLCtx.policyCtx.pRolePriority, pRoleName, &priority);

        g_array_append_val(g_AHLCtx.policyCtx.pInterruptBehavior, interupBehavior);

        // Sources
        GArray * pRoleSourceDeviceArray = g_array_new(FALSE, TRUE, sizeof(EndpointInfoT));
        g_ptr_array_add(g_AHLCtx.policyCtx.pSourceEndpoints,pRoleSourceDeviceArray);
        if (iNumInDevices) {          
            err = EnumerateSources(jInputDevices,i,pRoleName);
            if (err) {
                AFB_ERROR("Invalid input devices : %s", json_object_to_json_string(jInputDevices));
                return 1;
            }
        }
        // Sinks
        GArray * pRoleSinkDeviceArray = g_array_new(FALSE, TRUE, sizeof(EndpointInfoT));
        g_ptr_array_add(g_AHLCtx.policyCtx.pSinkEndpoints,pRoleSinkDeviceArray);
        if (iNumOutDevices) { 
            err = EnumerateSinks(jOutputDevices,i,pRoleName);
            if (err) {
                AFB_ERROR("Invalid output devices : %s", json_object_to_json_string(jOutputDevices));
                return 1;
            }
        }
        // Events
        GArray * pEventsArray = g_array_new(FALSE, TRUE, sizeof(GString));
        g_ptr_array_add(g_AHLCtx.policyCtx.pEventList,pEventsArray);
        // Parse and validate list of available events
        for (int i = 0; i < iNumEvents; i++)
        {
            json_object * jEvent = json_object_array_get_idx(jEvents,i);
            char * pEventName = (char *)json_object_get_string(jEvent);
            GString * gsEventName = g_string_new(pEventName);
            g_array_append_val(pEventsArray, *gsEventName);
        }
    }

    // Build lists of all device URI referenced in config file (input/output)    
    AFB_DEBUG ("Audio high-level - Parse high-level audio configuration done");
    return 0;
}
