# LSW v5

##### TL;DR:
- This is the CORE that you can add as submodule in your project.
- The development is done with [this respository](https://github.com/Lohkdesgds/LSW-DevTest).
- You can use this anywhere if you talk to me first. Please talk about your project on our [Discord server](https://discord.gg/JkzJjCG)! Don't be shy!
<br>

# About

*LSW is a huge project I've been working on that does eveything (or almost anything) for you. Need a easy socket connection? Check. Need a display? Check. Need collisions, images, animations? Check. Need to download a file? Check. Hash? Check.*

*Didn't get it yet? This is a combo of what I've done all this years in one package. This is the perfect combo for 2D graphics with tools. You can use only the tools if you want to. It should be THE TOOL to make good apps.*

*There's a big example [here](https://github.com/Lohkdesgds/LSW-DevTest). There are other projects within this account using this for multiple different scenarios.*
<br>

# How to get it running?

*You have to follow these steps to get a project up and running:*<br>
***PS:** Linux support will come in the future, but it's not my priority right now. You can help by joining the official [Discord Server](https://discord.gg/JkzJjCG) and talking about it.*
<br>
<hr>

### ✔ Windows 10, Visual Studio 2019:

- *Create a new project (I recommend Empty Project)*
- *Setup project's folder with git (git init) and add this as submodule (I recommend setting output next to the .vcxproj, on LSW/...)*
  - Example:
    - New Project "ABC", using default "create folders" thing: `git submodule add <link this> ABC/LSW`
    - Your .vcxproj should be at ABC/ABC.vcxproj
- *Open Nuget, browse for **Allegro** and install it (the dependency too, of course)*
- *Open project properties, then: (* ◼ *recommended,* ◻ *optional, but recommended )*
  * ◼ *General > C++ Language Standard: **C++17**;*
  * ◻ *General > C Language Standard: **C17**;*
  * ◼ *Allegro 5 > Add-ons: **"Yes"** on **all** items (easier);*
  * ◼ *Allegro 5 > Library Type: **"Dynamic Debug"** for **DEBUG** and **"Static Monolith"** for **RELEASE**;*
  * ◼ *C/C++ > Code Generation > Runtime Library: **/MT** and **/MTd** for **RELEASE** and **DEBUG**;*
  * ◻ *C/C++ > Code Generation > Floating Point Model: **Fast**;*
  * ◼ *Linker > General > Additional Libraries Directory: Add the **"LIB\x64"** to the **"x64"** and **"LIB\x86"** to the **"Win32"**;*
  * ◻ *VC++ Directories > Include Directories: Add **".\LSW\LSW"** path (or path to LSW folder in this) so you can do* **`#include <LSWv5/h>`** *anywhere.*

<hr>

#### Any problems, please contact:
- Twitter: @lohkdesgds
- Discord: [https://discord.gg/JkzJjCG](https://discord.gg/JkzJjCG) (I'll be there)
- Telegram: @lohkdesgds (probably offline most of the time)


#### FAQ

**🔹 Why V5 if it's V1.x?</h5>**<br>
⇾ *I called it V5 because it is the 5th try. The Vx.y.z thing is more like a V5.(x-1).y.z.*

**🔹 Does this work on x86 (32 bit) and/or x64 (64 bit) system?</h5>**<br>
⇾ *Yes, both.*

**🔹 Can I fork so I can modify the library to my needs?</h5>**<br>
⇾ *If you already have the permission to use it, yes. Please make your forks public though. You can also do pull request if you think this/these new feature(s) are useful.*