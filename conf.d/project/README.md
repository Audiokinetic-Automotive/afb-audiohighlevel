------------------------------------------------------------------------
                     Configuration
------------------------------------------------------------------------

#  ALSA Configuration
An example .asoundrc is provide with the file asoundrc-audio4a.
The example define 2 audio devices with 8 ALSA softvol control. The prefix of the softvol control and the virtual PCM device must match a support audio role.

For example:
```
pcm.Entertainment_Main {
 type softvol
 slave.pcm "SoftMixer"
 control{
   name "Entertainment_Volume"
   card 0
 }
}
```


Define a PCM Entertainment_Main device and Entertainment_Volume softvol control for the Entertainment audio role.
Please modify your /etc/asound.conf or ~/.asoundrc configuration to match your hardware audio configuration.

# AHL Configuration File
ahl-audio4a-config.json is an example of AHL configuration file.

Please modify the configuration file to match with your .asoundrc configuration and the number of Audio role needed.
Copy the file at a location following the rule describe below.

# AHL Configuration File location rule

At loading time the binding will search for a JSON configuration file located following theses rules:

- default search path is $PROJECT_ROOT/conf.d/project:${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}
- if environment variable "AAAA_CONFIG_PATH" is defined that it is used as search path
- config file name should match "ahl-BINDERNAME-config.json" where BINDERNAME is provided through "--name=BINDERNAME" in afb-daemon commande line.

Example:

```
export AAAA_CONFIG_PATH=~/opt/config
afb-daemon --name audio4a --workdir=.--ldpaths=./lib:../afb-aaaa/lib/afb-hal-intel-hda.so:../4a-alsa-core/lib/afb-alsa-4a.so --port=1234 --roothttp=./htdocs --token="" --verbose

```
#Configuration parameters definitions
--------------------------------------

hal_list: Define a list of HAL used with AHL. This is a list of HAL Binding API Name and it used by AHL to associate Endpoint with a corresponding HAL.

audio_roles: Define a specific set of Policy to be applied to a list of Endpoints.

Each audio roles has the following parameters:

name: Define the name of the audio role. The audio role name are define in ahl-policy/ahl-interface.h and use by the sample policy implementation.                  

Currently the following audio role name are available:

Warning          : Safety-relevant or critical alerts/alarms
Guidance         : Important user information where user action is expected (e.g. navigation instruction)
Notification     : HMI or else notifications (e.g. touchscreen events, speech recognition on/off,...)
Communication    : Voice communications (e.g. handsfree, speech recognition)
Entertainment    : Multimedia content (e.g. tuner, media player, etc.)
System           : System level content or development
Startup          : Early (startup) sound
Shutdown         : Late (shutdown) sound


priority         : Define the priority audio stream associate with the audio role. 

interupt_behavior: Define the policy applied when the current stream interrupt a lower or equal priority stream.

The following interrupt behavior are implemented in the sample policy engine:

- "continue" : Volume Ducking, the volume of the lower priority stream is lower. The value is defined inside the policy engine. A AHL_ENDPOINT_VOLUME_EVENT is generated.
- "pause"    : Stream paused, a AHL_STREAM_STATE_EVENT with state_event=STREAM_EVENT_PAUSE is generated to the lower priority stream.
- "cancel"   : Stream stop, a AHL_STREAM_STATE_EVENT with state_event=STREAM_EVENT_STOP is generated to the lower priority stream. 

Example:

An Entertainment application is playing music on the ALSA PCM Entertainment_Main (This PCM is associate to hw:0).
A Guidance app with a higher priority request a stream to be play on Guidance_Main (Guidance_Main is also associate with hw:0).
The Guidance audio role has the interrupt_behavior set to "continue".

The policy engine detect a Volume Ducking situation and the volume of Entertainment_Main is lower.
When the Guidance app stop or close it audio stream, the volume of Entertainment_Main is restore back to it initial value.

output/input: Define the list of sink/source endpoints available for the audio role.

The endpoint PCM URI value are define with the following rule:

framework.pcm_name

Example:

alsa.hw:0
alsa.plug:Entertainment_Main
pulse.default

actions : Define the list of actions support for the audio role.

Currently not implemented, this is a provision in the configuration file for future use case such as sound generation.


