<img src="https://cdn.discordapp.com/attachments/739704685505544363/884649384543940608/lunaris_logo_by_lupspie.png" width="400" align="right">

# Lunaris
*The multi-purpose graphics, audio, socket and so much more library combo based on Allegro 5 created by Lohk!*

<br>

#### Useful information about this version (automatically built)
Key | Value
--- | ---
LUNARIS_BUILD_NUMBER | 1929

<br>

##### TL;DR:
- You can add this as a submodule in your project. You just have to add [Allegro5](https://github.com/liballeg/allegro5) via NuGet (preferable).
- You can use this anywhere if you talk to me first. Please talk about your project on our [Discord server](https://discord.gg/eQfnXdTH6f)! Don't be shy!
<br>

# About

*Lunaris is a huge project I've been working on that does eveything (or almost anything) for you. Need a easy socket connection? Check. Need a display? Check. Need collisions, images, animations? Check. Need to download a file? Check. Hash? Check.*

*Didn't get it yet? This is a combo of what I've done all this years in one package. This is the perfect combo for 2D graphics with tools. You can use only the tools if you want to. It should be THE TOOL to make good apps.*

*Big examples coming soon!*
<br>

# How to get it running?

*You have to follow these steps to get a project up and running:*<br>
***PS:** Linux support will come in the future, but it's not my priority right now. Some stuff already works on Linux. You can help by joining the official [Discord Server](https://discord.gg/eQfnXdTH6f) and talking about it.*
<br>
<hr>

### ✔ Windows 10, Visual Studio:

- *Create a new project (I recommend Empty Project)*
- *Setup project's folder with git (git init) and add this as submodule (I recommend setting output next to the .vcxproj, on Lunaris/...)*
  - Example:
    - New Project "ABC", using default "create folders" thing: `git submodule add <link this> Lunaris`
    - Your .vcxproj should be at ABC/ABC.vcxproj
- *Open Nuget, browse for **Allegro** and install it (the dependency too, of course)*
- *Open project properties, then: (* ◼ *recommended,* ◻ *optional, but recommended )*
  * ◼ *General > C++ Language Standard: **C++17**;*
  * ◻ *General > C Language Standard: **C17**;*
  * ◼ *Allegro 5 > Add-ons: **"Yes"** on **all** items (easier);*
  * ◼ *Allegro 5 > Library Type: **"Dynamic Debug"** for **DEBUG** and **"Static Monolith"** for **RELEASE**;*
  * ◼ *C/C++ > Code Generation > Runtime Library: **/MT** and **/MTd** for **RELEASE** and **DEBUG**;*
  * ◻ *C/C++ > Code Generation > Floating Point Model: **Fast**;*
  * ◼ *Linker > General > Additional Libraries Directory: Add **"$(SolutionDir)Lunaris\libs\Lunaris\x64"** to the **"x64"** and **$(SolutionDir)Lunaris\libs\Lunaris\x86** to the **"Win32"**;*
  * ◻ *VC++ Directories > Include Directories: Add **$(SolutionDir)Lunaris\include** path (or path to Lunaris folder in this) so you can do* **`#include <Lunaris/module.h>`** *(module being one of the options like utility or graphics) anywhere.*
  * ◻ *C/C++ > Additional Include Directories: Add **$(SolutionDir)Lunaris\include** path (like the other one)*

<hr>

#### Any problems, please contact:
- Twitter: @lohkdesgds
- Discord: [https://discord.gg/JkzJjCG](https://discord.gg/eQfnXdTH6f) (I'll be there)
- Telegram: @lohkdesgds (probably offline most of the time)


#### FAQ

**🔹 What happened to LSW?**<br>
⇾ *I want a better name for this project so people can call it something cool, so Lunaris it is!*

**🔹 Does this work on x86 (32 bit) and/or x64 (64 bit) system?</h5>**<br>
⇾ *Yes, both.*

**🔹 Can I fork so I can modify the library to my needs?</h5>**<br>
⇾ *If you already have the permission to use it, yes. Please make your forks public though. You can also do pull request if you think this/these new feature(s) are useful.*