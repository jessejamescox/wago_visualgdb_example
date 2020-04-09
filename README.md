# wago_visualgdb_example
Example project using VisualGDB for Linux development on PfC and TP platform

# setting up the devices
When building your project use the following settings in the device setup:

| --- | Directory |
| --- | --- |
| `gcc` | /home/wago/wago/ptxproj/selected_toolchain/arm-linux-gnueabihf-gcc |
| `g++` | /home/wago/wago/ptxproj/selected_toolchain/arm-linux-gnueabihf-g++ |
| `gdb` | /home/wago/wago/ptxproj/selected_toolchain/arm-linux-gnueabihf-gdb |

Then add the Extra Path:
|  | Directory |
| --- | --- |
| Extra Path Directories | /home/wago/wago/ptxproj/platform-wago-PFCXXX/sysroot-target |

# setting up the Makiefile
You will need to add the include and library paths, and call the libraries

|  | Include Paths |
|  | /home/wago/wago/ptxproj/platform-wago-pfcXXX/sysroot-target/include  |
|  | /home/wago/wago/ptxproj/platform-wago-pfcXXX/sysroot-target/usr/include  |
|  | /home/wago/wago/ptxproj/platform-wago-pfcXXX/sysroot-target/usr/include/OsLinux |

|  | Library Paths |
|  | /home/wago/wago/ptxproj/platform-wago-pfcXXX/sysroot-target/usr/lib  |
|  | //home/wago/wago/ptxproj/platform-wago-pfcXXX/sysroot-target/lib  |

|  | Library Names |
|  | dal |
|  | glib-2.0 |
|  | pthread |
|  | ffi |
|  | rt |
|  | libloader |
|  | oslinux |
|  | dl |
|  | ncurses |
|  | pcre |
