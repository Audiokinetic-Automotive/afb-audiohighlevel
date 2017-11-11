------------------------------------------------------------------------
                  AGL Audio High Level Binding
------------------------------------------------------------------------

# Audio High Level Binding
-----------------------------
The Audio High Level Binding is the upper layer in the Audio 4A architecture.
The binding provide a simple, unified single entry point for all AGL audio applications.

Here are the features provide by the binding:

- Expose all audio device capabilities in uniform way to applications
- Provide applications display name for device (e.g. UI selection)
- Provide applications device URI to stream to selected endpoint
- Automatically retrieve associated volume control for ALSA softvol URI
- Allow fine grain security permissions control, policy enforcement and application audio stream control isolation
- Priority-based and audio role specific endpoint selection / stream routings (automatic or explicit) and aggregation of different audio domains (ALSA, Pulse)
- Audio stream controls (volume, mute, state, properties)


# Glossary
-----------------------------
The binding define the following term:

- AHL                : Audio High Level Binding
- Audio role         : Specific set of audio policy rules applied to a group of audio stream
- Endpoint           : Virtual audio sink or source device.
- Stream             : Audio connection between a source and sink.
- Audio 4A Framework : AGL Audio Framework using a set of low level, HAL and HLB bindings.
- Policy Engine      : Static library define in ahl-policy.c implementing audio policy.


# Policy Engine
------------------------------

The current implementation of the sample policy engine is a static library.
The interface between the policy engine and the audio high level binding is a simple JSON interface.
This allow user to easily replace it with their own policy engine.


# Endpoint Selection
------------------------------
AHL JSON configuration file define a number of possible endpoint per audio role. The EndPoint are list in order of priority, from highest to lowest priority.
At init time, AHL will validate each endpoint and only keep a list of active one per audio role. Inactive endpoint are discard and is not accessible to application.

Application can request a list of available Endpoint for a specific audio role by calling the API/Verb get_endpoints.
The application can decide to open a stream on a specific endpoint from the list by specifying an EndpointID or let AHL assigned an endpoints based on it audio role.
In the latter case it will be the first endpoint on the list return by get_endpoints.

# Events
-------------------------------
Currently AHL will generate 4 types of events, they are define in ahl-interface.h.
They are the following:

- AHL_STREAM_STATE_EVENT: Application are automatically susbcribe to this event. They will only received event of stream they have opened.
- AHL_ENDPOINT_VOLUME_EVENT: Application need to subscribe to this event to receive volume change notification.
- AHL_ENDPOINT_PROPERTY_EVENT: Application need to subscribe to this event to receive Property Change notification.
- AHL_POST_ACTION_EVENT: Application need to subscribe to this event to receive an Action change notification. (This is for future use case, sound generation for example)


# AHL Configuration File and System configuration
----------------------------------------------
Please refers to https://github.com/Audiokinetic-Automotive/afb-audiohighlevel/blob/master/conf.d/project/README.md


# Cloning Audio High Level from Git
-------------------------------------------------------

```
# Initial clone with submodules
git clone --recurse-submodules https://github.com/Audiokinetic-Automotive/afb-audiohighlevel.git
cd  audio-binding

# Do not forget submodules with pulling
git pull --recurse-submodules https://github.com/Audiokinetic-Automotive/afb-audiohighlevel.git

```

# System libraries Dependencies
------------------------------------------------------------------
- libasound (version 1.1.2 or latest)
- libsystemd (version 222 or latest)
- libmicrohttpd (version 0.9.55 or latest)
- afb-daemon (version 2.0 or latest)
- json-c
- libafbwsc
- glib-2.0

# AGL Binding dependencies
-------------------------------------------------------------------
AGL Audio High Level Binding is part of the AGL Audio 4A framework,
It required the following AGL bindings:

- 4a-alsa-core: Alsa Low Level Bindings
  source: https://gerrit.automotivelinux.org/gerrit/#/admin/projects/src/4a-alsa-core
- 4a-hal-reference: Hardware Abstraction Layer Binding
source: https://gerrit.automotivelinux.org/gerrit/#/admin/projects/src/4a-hal-reference   


# Compile AGL Audio High Level Binding
--------------------------------------

Set INSTALL_PREFIX variable to your local AGL binding install folder.


```
export INSTALL_PREFIX=~/opt
mkdir -p build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX ..
make
make install

```

# Launch command and usage

```
afb-daemon --name audio4a --workdir=.--ldpaths=./lib:../4a-hal-reference/lib/afb-hal-intel-hda.so:../4a-alsa-core/lib/afb-alsa-4a.so --port=1234 --roothttp=./htdocs --token="" --verbose

```
