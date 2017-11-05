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

#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "wrap-json.h"
#include "filescan-utils.h"

#include "ahl-binding.h"

extern AHLCtxT g_AHLCtx;

static InterruptBehaviorT InterruptBehaviorToEnum(char * in_pInterruptBehaviorStr)
{
    g_assert_nonnull(in_pInterruptBehaviorStr);
    if (strcasecmp(in_pInterruptBehaviorStr,AHL_INTERRUPTBEHAVIOR_CONTINUE)==0) {
        return INTERRUPTBEHAVIOR_CONTINUE;
    }
    else if (strcasecmp(in_pInterruptBehaviorStr,AHL_INTERRUPTBEHAVIOR_CANCEL)==0) {
        return INTERRUPTBEHAVIOR_CANCEL;
    }
    else if (strcasecmp(in_pInterruptBehaviorStr,AHL_INTERRUPTBEHAVIOR_PAUSE)==0) {
        return INTERRUPTBEHAVIOR_PAUSE;
    }
    else 
        return INTERRUPTBEHAVIOR_MAXVALUE;
}

static json_object* CtlConfigScan(const char *dirList, const char *prefix) {
    char controlFile [CONTROL_MAXPATH_LEN];
    strncpy(controlFile, prefix, CONTROL_MAXPATH_LEN);
    strncat(controlFile, GetBinderName(), CONTROL_MAXPATH_LEN);

    // search for default dispatch config file
    json_object* responseJ = ScanForConfig(dirList, CTL_SCAN_RECURSIVE, controlFile, "-config.json");

    return responseJ;
}

static char* CtlConfigSearch(const char *dirList, const char *prefix) {
    int index, err;

    // search for default dispatch config file
    json_object* responseJ = CtlConfigScan (dirList, prefix);
    if (!responseJ) return NULL;

    // We load 1st file others are just warnings
    for (index = 0; index < json_object_array_length(responseJ); index++) {
        json_object *entryJ = json_object_array_get_idx(responseJ, index);

        char *filename;
        char*fullpath;
        err = wrap_json_unpack(entryJ, "{s:s, s:s !}", "fullpath", &fullpath, "filename", &filename);
        if (err) {
            AFB_ERROR("CTL-INIT HOOPs invalid JSON entry= %s", json_object_get_string(entryJ));
            return NULL;
        }

        if (index == 0) {
            char filepath[CONTROL_MAXPATH_LEN];
            strncpy(filepath, fullpath, sizeof (filepath));
            strncat(filepath, "/", sizeof (filepath));
            strncat(filepath, filename, sizeof (filepath));
            return (strdup(filepath));
        }
    }
    // no config found
    return NULL;
}

int ParseHLBConfig() {
    char * versionStr = NULL;
    json_object * jAudioRoles = NULL;
    json_object * jHALList = NULL;
    char * policyModule = NULL;
    
    const char *dirList=getenv("AAAA_CONFIG_PATH");
    if (!dirList) dirList=CONTROL_CONFIG_PATH;
    
    const char *configfile_path =CtlConfigSearch(dirList, "ahl-");
    if (!configfile_path) {
        AFB_ERROR("Error: No control-* config found invalid JSON %s ", dirList);
        return AHL_FAIL;
    }

    AFB_NOTICE("High-level config file -> %s\n", configfile_path);

    // Open configuration file
    json_object *config_JFile=json_object_from_file(configfile_path);
    if(config_JFile == NULL)
    {
        AFB_ERROR("Error: Can't open configuration file -> %s",configfile_path);
        return AHL_FAIL;
    }

    int err = wrap_json_unpack(config_JFile, "{s:s,s:s,s:o,s:o}", "version", &versionStr,"policy_module", &policyModule,"audio_roles",&jAudioRoles,"hal_list",&jHALList);
    if (err) {
        AFB_ERROR("Invalid configuration file -> %s", configfile_path);
        return AHL_FAIL;
    }
    AFB_INFO("High-level audio API version: %s", "1.0.0");
    AFB_INFO("Config version: %s", versionStr);
    AFB_INFO("Policy module: %s", policyModule);

    int iHALListLength = json_object_array_length(jHALList);
    g_AHLCtx.policyCtx.pHALList = g_ptr_array_new_with_free_func(g_free);
    int iNumberOfRoles = json_object_array_length(jAudioRoles);
    g_AHLCtx.policyCtx.pRoleInfo = g_hash_table_new(g_str_hash, g_str_equal);
        
    for (int i = 0; i < iHALListLength; i++)
    {
        char * pHAL = NULL;
        json_object * jHAL = json_object_array_get_idx(jHALList,i);
        if (jHAL) {
            pHAL = (char *)json_object_get_string(jHAL);
            char * pHALName = g_strdup( pHAL );
            g_ptr_array_add( g_AHLCtx.policyCtx.pHALList, pHALName );

            // Set dependency on HAL specified
            err = afb_daemon_require_api_v2(pHAL,1) ;
            if( err != 0 )
            {
                AFB_ERROR("Audio high level API could not set dependency on API: %s",pHAL);
                return AHL_FAIL;
            }
        }
    }

    for (int i = 0; i < iNumberOfRoles; i++)
    {
        int priority = 0;
        json_object * jAudioRole = json_object_array_get_idx(jAudioRoles,i);
        json_object * jOutputDevices = NULL;
        json_object * jInputDevices = NULL;
        json_object * jActions = NULL;
        char * pRoleName = NULL;
        char * pInteruptBehavior = NULL;

        int iNumOutDevices = 0;
        int iNumInDevices = 0;
        int iNumActions = 0;

        err = wrap_json_unpack(jAudioRole, "{s:s,s:i,s:s,s?o,s?o,s?o}", 
                                    "name", &pRoleName,
                                    "priority",&priority,
                                    "interupt_behavior",&pInteruptBehavior,
                                    "output",&jOutputDevices,
                                    "input",&jInputDevices,
                                    "actions",&jActions
                                    );
        if (err) {
            AFB_ERROR("Invalid audio role configuration : %s", json_object_to_json_string(jAudioRole));
            return AHL_FAIL;
        }
        
        if (jOutputDevices)
            iNumOutDevices = json_object_array_length(jOutputDevices);
        if (jInputDevices)
            iNumInDevices = json_object_array_length(jInputDevices);
        if (jActions)
            iNumActions = json_object_array_length(jActions);

        RoleInfoT * pRoleInfo = (RoleInfoT*) malloc(sizeof(RoleInfoT));
        memset(pRoleInfo,0,sizeof(RoleInfoT));
        pRoleInfo->pRoleName = g_strdup( pRoleName );
        pRoleInfo->iPriority = priority;
        pRoleInfo->eInterruptBehavior = InterruptBehaviorToEnum(pInteruptBehavior);

        // Actions
        pRoleInfo->pActionList = g_ptr_array_new_with_free_func(g_free);
        // Parse and validate list of available actions
        for (int i = 0; i < iNumActions; i++)
        {
            json_object * jAction = json_object_array_get_idx(jActions,i);
            char * pActionName = (char *)json_object_get_string(jAction);
            if (pActionName)
                g_ptr_array_add(pRoleInfo->pActionList, g_strdup(pActionName));
        }

        // Sources
        pRoleInfo->pSourceEndpoints = g_ptr_array_new_with_free_func(g_free);
        if (iNumInDevices) {          
            err = EnumerateDevices(jInputDevices,pRoleName,ENDPOINTTYPE_SOURCE,pRoleInfo->pSourceEndpoints);
            if (err) {
                AFB_ERROR("Invalid input devices : %s", json_object_to_json_string(jInputDevices));
                return AHL_FAIL;
            }
        }
        // Sinks
        pRoleInfo->pSinkEndpoints = g_ptr_array_new_with_free_func(g_free);
        if (iNumOutDevices) { 
            err = EnumerateDevices(jOutputDevices,pRoleName,ENDPOINTTYPE_SINK,pRoleInfo->pSinkEndpoints);
            if (err) {
                AFB_ERROR("Invalid output devices : %s", json_object_to_json_string(jOutputDevices));
                return AHL_FAIL;
            }
        }

        g_hash_table_insert(g_AHLCtx.policyCtx.pRoleInfo, pRoleInfo->pRoleName, pRoleInfo);
    }

    // Build lists of all device URI referenced in config file (input/output)    
    AFB_DEBUG ("Audio high-level - Parse high-level audio configuration done");
    return AHL_SUCCESS;
}
