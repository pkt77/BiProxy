BiProxy For Minecraft
=
This project aims to be the fastest proxy software for both Java and Bedrock edition Minecraft servers.
While other proxies are usually written in Java and work for either edition, this proxy aims to handle player traffic for Minecraft networks that want to support both editions.

Disclaimer
-
I am in no way a C++ professional, but I have been writing Java for about 10 years and have always wanted to work on a C++ project. So, this is more of a learning project for me in my free time than it is a reliable working one. I do not expect to complete this on my own, however I do enjoy this and would like to make progress often.

Goals & Progress
-
Currently this software only handles ping packets for both editions and responds with the MOTD.

Next goal: handle login packets and deny them (as a test)

I'm writing this on Windows, so this project doesn't support Unix systems just yet.

The end goal is to support the latest versions without cross-play, but I'd like to add support for most, if not all, modern versions and allow some sort of cross-play.

I'd like to make some sort of API, maybe even Java plugin support for those that don't know C++

####BungeeCord
I plan to mimic the BungeeCord protocol so there will be no need to write a custom server jar.

I also plan to use the same plugin channel messages so server plugins can interact with the proxy without needing additional code.

Contributions & Resources
-
If you'd like to contribute, please format your code similar to Java conventions and this project.

I use the following pages to translate the protocol to my design:
- [Java Edition Protocol](https://wiki.vg/Protocol)
- [Bedrock Edition Protocol](https://wiki.vg/Bedrock_Protocol)
- [RakNet Protocol](https://wiki.vg/Raknet_Protocol)

I also use the BungeeCord and Nukkit projects as references and inspiration.