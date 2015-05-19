# anope with EkiNetIrc patches
Anope IRC Services customized by EkiNetIrc

## Installation
1) Clone latest stable tag from the repo and `cd` to it:
```
$ git clone https://github.com/EkiNetIrc/anope.git -b 2.0.2+EkiNetIrc && cd anope
```

2) Run:
```
$ ./extras
```
and enable `m_mysql.cpp`, `m_sasl_dh-aes.cpp`, `m_sasl_dh-blowfish.cpp` & `m_ssl_openssl.cpp`.


3) Launch the Config script:
```
$ ./Config
```
Here is an copy of an production installation:
```
Beginning Services configuration.

In what directory should Anope be installed?
[/home/irc/services]

Which group should all Services data files be owned by?  (If Services
should not force files to be owned by a particular group, just press
Return.)
[] irc

What should the default umask for data files be (in octal)?
(077 = only accessible by owner; 007 = accessible by owner and group)
[007] 077

Would you like to build a debug version of Anope?
[n]

You can optionally have the build run through run-cc.pl, which will
cause warnings and errors (if any) to be colored yellow and red,
respectively. This relies on Perl being installed, so if you say yes
to this without Perl, the option will be ignored.
NOTE: If you are using MinGW, it is NOT recommended to say yes to
this, it may fail.
Would you like to utilize run-cc.pl?
[n]

Do you want to build using precompiled headers? This can speed up
the build, but uses more disk space.
[n]

Are there any extra include directories you wish to use?
You may only need to do this if CMake is unable to locate
missing dependencies without hints.
Separate directories with semicolons.
If you need no extra include directories, enter NONE in all caps.
[] NONE

Are there any extra library directories you wish to use?
You may only need to do this if CMake is unable to locate
missing dependencies without hints.
Separate directories with semicolons.
If you need no extra library directories, enter NONE in all caps.
[] NONE

Are there any extra arguments you wish to pass to CMake?
If you need no extra arugments to CMake, enter NONE in all caps.
[] NONE
```

4) Start the compilation process:
```
$ cd build && make
```

5) During the compilation, check if configuration files has changed & apply changes.

6) Prepare the network for a small services interruption.

7) Drink a coffe when waiting the end of the compilation.

8) Install files:
```
$ make install
```

9) Restart Anope & pray!
