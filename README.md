------------------------------------------------------------------------
                  AGL Audio High Level Binding
------------------------------------------------------------------------

# Cloning Audio High Level from Git
-------------------------------------------------------

```
# Initial clone with submodules
git clone --recurse-submodules https://github.com/Audiokinetic-Automotive/afb-audiohighlevel.git
cd  audio-binding

# Do not forget submodules with pulling
git pull --recurse-submodules https://github.com/Audiokinetic-Automotive/afb-audiohighlevel.git

```

#  AFB-daemon dependencies
-------------------------------------------------------

    OpenSuse >=42.2, Fedora>=25, Ubuntu>=16.4 Binary packages from  https://en.opensuse.org/LinuxAutomotive

    For other distro see # Building AFB-daemon from source on Standard Linux Distribution

 
# Specific Dependencies 

 * alsa-devel >= 1.1.2 Warning some distro like Fedora-25 still ship version 1.1.1 as default
 * lua >= 5.3  Most distribution only ship version 5.2 but binary package should be easy to find

On Ubuntu 16.4 you should recompile AlsaLib from source ftp://ftp.alsa-project.org/pub/lib/
as today latest stable is 1.1.4. 


```
  OpenSuse
     - LUA-5.3-devel  https://software.opensuse.org//download.html?project=devel%3Alanguages%3Alua&package=lua53
     - Alsa-devel zypper --install alsa-devel # 42.3 is shipped default with 1.1.4 

  Fedora 26 (out of the box)
     - Lua 5.3 
     - Alsa-devel 1.1.4

  Ubuntu-16.4
     - LUA-5.3 is avaliable in binary through apt-get
     - Alsa should be recompiled from source
 
        wget ftp://ftp.alsa-project.org/pub/lib/alsa-lib-1.1.4.1.tar.bz2
        tar -xjf alsa-lib-1.1.4.1.tar.bz2
        cd alsa-lib-1.1.4.1
        ./configure --prefix=/opt

  Ubuntu-17.04 (out of the box)
     - Alsa 1.1.4
     - Lua 5.3 

  #WARNING: do not forget to upgrade your PKG_CONFIG_PATH=/opt/lib/pkgconfig or whatever is the place where
  your installed alsa/lua.
```


# Compile AGL Audio High Level Binding
--------------------------------------

* Edit your ~/.config/app-templates/cmake.d/00-common-userconf.cmake to reflect your local configuration

```
    message(STATUS "*** User Local Config For Native Linux ***")
    add_compile_options(-DNATIVE_LINUX)
    
    set(CMAKE_INSTALL_PREFIX $ENV{HOME}/opt)
    set(BINDINGS_INSTALL_PREFIX $ENV{HOME}/opt)

    set(CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX}/lib64/pkgconfig ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
    set(LD_LIBRARY_PATH ${CMAKE_INSTALL_PREFIX}/lib64 ${CMAKE_INSTALL_PREFIX}/lib)

```


```
    mkdir -p build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX ..
    make
    make install


    afb-daemon --workdir=. --ldpaths=.. --port=1234  --roothttp=./htdocs --token="" --verbose
    


