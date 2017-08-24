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
#include <time.h>

#include "ahl-binding.h"

PUBLIC int EnumerateSources() {
    
    // TODO: Use lower level services to build a list of available source devices
    
    AFB_DEBUG ("Audio high-level - Enumerate sources done");
    return 0;
}

PUBLIC int EnumerateSinks() {
    
    // TODO: Use lower level services to build a list of available sink devices
    
    AFB_DEBUG ("Audio high-level - Enumerate sinks done");
    return 0;
}
 
