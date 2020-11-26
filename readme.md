# LSW v5

- This is the CORE that you can add as submodule in your project
- The development is done with [this respository](https://github.com/Lohkdesgds/LSW-DevTest).

---

*You have to follow these steps to get a project up and running:*

---

### ⇒ *WINDOWS - VISUAL STUDIO*

- Create a new project (I recommend Empty Project)
- Put this folder somewhere you can #include the LSWv5.h
- Open Nuget, browse for *Allegro* and install (both it and the dependency)
- Open project properties, then:
  * General > C++ Language Standard: C++17
  * General > C Language Standard: C17
  * Allegro 5 > Add-ons: "Yes" on all items (easier)
  * Allegro 5 > Library Type: "Dynamic Debug" for DEBUG and "Static Monolith" for RELEASE
  * C/C++ > Code Generation > Runtime Library: **/MT** and **/MTd** for RELEASE and DEBUG
  * C/C++ > Code Generation > Floating Point Model: Fast (recommended)
  * Linker > General > Additional Libraries Directory: Add the "LIB\x64" to the "x64" and "LIB\x86" to the "Win32"

#### Any problems, please contact:
- Twitter: @lohkdesgds


### This page still under development