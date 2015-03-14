# Introduction #

YPlatform2 is designed to be **modular** and can run multiple applets (modules) providing numerous **API calls** for communication (filesystem, cvars, commands, renderer). This way, in most cases, modules do not need to be recompiled on each API change. This also means less code since renderer is detached from the platform's code.


# Details #

Example module source code is in the trunk. To create your first module, copy the folder **modules/module** and modify it to your needs.
All calls to the platform are made via **Mod\_Trap** external class (e.g. _mt.comPrint( "Hello world" );_ will output this string to the main console). See **modules/mod\_trap.h** for more API calls.
Platform communicates with the module via **modMain** function which is mandatory for every module.
Modules are updated with 50 FPS.
YPlatform2 also supports multiple renderers in a similar way.

Each module requires a **manifest** for it to be loaded. Manifest is an **XML** configuration file in **modules/** directory either packaged or in **homeDir**.
**Syntax example:**
```
<module name="Super Awesome Module" type="module">
  <description>Does nothing</description>
  <filename>superAwesome</filename>
  <icon>icons/superAwesome.png</icon>
  <api>1</api>
</module>
```

  * Name and filename tag is **mandatory**.
  * Type is either **module** or **renderer** (**soundSystem** is to be added in future releases).
  * Filename is the base filename of the module library. Platform looks for module named _filename\_platform\_arch.extension_ (e.g. _libRenderer\_linux\_i386.so_) in **modules/** directory.
  * Modules can be packaged and are automatically **extracted**.
  * On windows platform, modules can also be linked with a **shortcut**.

Applets can be loaded from GUI or via _mod\_load "moduleName"_ command either from the main window or through startup arguments with addition of plus sign (_+mod\_load_).
