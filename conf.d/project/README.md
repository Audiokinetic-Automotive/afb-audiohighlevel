
1) $HOME/.asoundrc or /etc/asound.conf should match you hardware audio configuration.
2) ahl-xxxxxx-config.json should match your .asoundrc config

Note: config file is search within with following rules:
 - default search path is $PROJECT_ROOT/conf.d/project:${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}
 - if environment variable "AAAA_CONFIG_PATH" is defined that it is used as search path
 - config file should match "ahl-BINDERNAME-config.json" where BINDERNAME is provided through "--name=BINDERNAME" in afb-daemon commande line.

Note: you may debug Audio-4A from your development tree with:

  afb-daemon --name=afb-audio4a --port=1234 --ws-server=unix:/var/tmp/ahl-4a --cntxtimeout=1 \
             --alias=/monitoring:/home/fulup/opt/afb-monitoring --binding=package/lib/afb-audiohighlevel.so \
             --ldpaths=../../alsa-4a/build/package/lib:../../hal-sample-4a/build/package/lib --workdir=. \
             --roothttp=../htdocs --token= --verbose --verbose