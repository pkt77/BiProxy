BiProxy For Minecraft
=
This project aims to be the fastest proxy software for both Java and Bedrock edition Minecraft servers.
While other proxies are usually written in Java and work for either edition, this proxy aims to handle player traffic for Minecraft networks that want to support both editions.

Disclaimer
-
I am in no way a C++ professional, but I have been writing Java for about 10 years and have always wanted to work on a C++ project. So, this is more of a learning project for me in my free time than it is a reliable working one. I do not expect to complete this on my own, however I do enjoy this and would like to make progress often.

Goals & Progress
-
Currently this software:
- Handles ping packets for both editions and responds with the MOTD
- Handles logins for Java edition
- Logs in (offline mode) and passes Java packets to a single local server
- Handle bedrock login packets and kicks for "Outdated server" for now

Next goal: Clean up code and remove hard-coded values to support multiple players

For bedrock, there's 2 ways to handle players: Proxy all packets same as Java edition or use the Transfer packet in which the proxy just keeps track of servers and players. Might make this an option. The latter would require a server plugin, while the former *might* be optional, for features such as IP forwarding and plugin messages (depending on how clients authenticate logins with servers).

Currently will only be supporting the latest protocol versions (1.16). Eventually will get into supporting past versions.

I'm writing this on Windows, so this project doesn't support Unix systems just yet.

The end goal is to support the latest versions without cross-play, but I'd like to add support for most, if not all, modern versions and allow some sort of cross-play.

I'd like to make some sort of API, maybe even Java plugin support for those that don't know C++

#### BungeeCord
I plan to mimic the BungeeCord protocol so there will be no need to write a custom server jar.

I also plan to use the same plugin channel messages so server plugins can interact with the proxy without needing additional code.

How To Compile From Source
-
### Windows
- Install [vcpkg](https://vcpkg.io/en/getting-started.html) and [Visual Studio](https://visualstudio.microsoft.com/).
Unfortunately, those are required to install certain libs, unless someone has a work-around?
- Run `vcpkg install cpprestsdk cpprestsdk:x64-windows`
- Run `vcpkg install openssl-windows:x64-windows`
- Add `-DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake` to your CMake options

Contributions & Resources
-
If you'd like to contribute, please format your code similar to Java conventions and this project.

I use the following pages to translate the protocol to my design:
- [Java Edition Protocol](https://wiki.vg/Protocol)
- [Bedrock Edition Protocol](https://wiki.vg/Bedrock_Protocol)
- [RakNet Protocol](https://wiki.vg/Raknet_Protocol)

I also use the BungeeCord and Nukkit projects as references and inspiration.
