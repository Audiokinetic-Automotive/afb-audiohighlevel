------------------------------------------------------------------------
                     Configuration
------------------------------------------------------------------------

#  ALSA Configuration
An example .asoundrc is provided with the file asoundrc-audio4a. In this configuration, we choose to use software mixing of several virtual audio devices with distinct software volume controls (one per audio role). The example defines 2 audio zones with several ALSA virtual audio devices (endpoints) that applications should target. The prefix of the softvol control must match the configuration audio role name to automatically use audio role specific volume ramping and controls.

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

Defines a PCM Entertainment_Main endpoint using Entertainment_Volume softvol control for the Entertainment audio role volume.
Please modify your /etc/asound.conf or ~/.asoundrc configuration to match your hardware audio configuration.

# AHL Configuration File
*ahl-audio4a-config.json* is an example of an AHL configuration file.

Please modify the configuration file to match with your *.asoundrc* configuration and the desired audio roles.
Copy the file at a location as described below.

# AHL Configuration File Location

At loading time the AHL binding will search for a JSON configuration file located following theses rules:

- default search path is $PROJECT_ROOT/conf.d/project:${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}
- if environment variable "AAAA_CONFIG_PATH" is defined, it is used as search path
- configuration file name should match "ahl-BINDERNAME-config.json" where BINDERNAME is provided through "--name=BINDERNAME" in afb-daemon command line.

Example:

```
export AAAA_CONFIG_PATH=~/opt/config
afb-daemon --name audio4a --workdir=.--ldpaths=./lib:../afb-aaaa/lib/afb-hal-intel-hda.so:../4a-alsa-core/lib/afb-alsa-4a.so --port=1234 --roothttp=./htdocs --token="" --verbose

```
# Configuration parameters definitions
--------------------------------------

#### hal_list

 Define a list of HAL to be used with AHL. This is a list of HAL binding API names and it used by AHL to associate audio endpoints with a corresponding HAL.

#### audio_roles

Defines an application role specific (e.g. entertainment, navigation, etc.) list of prioritized endpoints, priorities and behaviors (e.g. interrupt) to be applied by the audio policy.

Each audio role has the following parameters:

- **name**

   Defines the name of the audio role. Some standard audio role names are provided in ahl-policy/ahl-interface.h and used in the sample policy implementation.                  

   In the sample configuration file (and accompanying policy implementation), the following audio role name are used:

   - **Warning**          : Safety-relevant or critical alerts/alarms
   - **Guidance**         : Important user information where user action is expected (e.g. navigation instruction)
   - **Notification**     : HMI or else notifications (e.g. touchscreen events, speech recognition on/off,...)
   - **Communication**    : Voice communications (e.g. handsfree, speech recognition)
   - **Entertainment**    : Multimedia content (e.g. tuner, media player, etc.)
   - **System**           : System level content or development
   - **Startup**          : Early (startup) sound
   - **Shutdown**         : Late (shutdown) sound


- **priority**         
Defines the priority audio stream associated with the audio role (will be used by policy implementation to determine audio focus).

- **interupt_behavior**

   Defines what happens when the current stream interrupts a lower or equal priority stream.

   The following interrupt behaviors are implemented in the sample policy engine:

   - "continue" : Volume ducking, the volume of the lower priority stream is lowered. The target volume value is defined by the policy engine. An AHL_ENDPOINT_VOLUME_EVENT volume event is generated.

   - "pause"    : Stream paused, a AHL_STREAM_STATE_EVENT with state_event=STREAM_EVENT_PAUSE is generated for the lower priority stream(s).

   - "cancel"   : Stream stop, a AHL_STREAM_STATE_EVENT with state_event=STREAM_EVENT_STOP is generated for the lower priority stream(s).

  **Example**

   An entertainment application is playing music on the ALSA PCM 'Entertainment_Main' (this PCM is routed to the software mixer targeting hw:0).
   A navigation application with a higher priority request a stream to be played on the ALSA PCM 'Guidance_Main' (this PCM is also routed to the software mixer targeting hw:0).
   The guidance audio role has the interrupt_behavior set to "continue".

   The policy engine implements a volume ducking situation and the software volume control associated with 'Entertainment_Main' is lowered during the navigation application playback.
   When the navigation application stops or closes its audio stream, the volume of 'Entertainment_Main' is restored back to it original value.

- **output/input**

  Defines the list of sink/source endpoints available for the audio role (in order of priority for automatic endpoint selection purposes).

  The endpoint PCM URI values use the following naming convention:

  *framework.pcm_name*

  **Example**:

  ```
  alsa.hw:0
  alsa.plug:Entertainment_Main
  pulse.default

  ```

- **actions**

  Defines the list of sound related actions supported for the audio role.

  Currently not implemented, this is a provision in the configuration file for future use case such as sound generation.
