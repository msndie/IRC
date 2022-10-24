# IRC

Simple IRC server written on C++, based on RFCs 1459, 2812, 2813. This server does not support server-server communication. Tested clients: Weechat, Adium.

<details>
<summary>IRCserv run</summary>
<img src="/screenshots/ircserv.png">
</details>
<details>
<summary>Joker-bot run</summary>
<img src="/screenshots/joker.png">
</details>
<details>
<summary>nc</summary>
<img src="/screenshots/nc.png">
</details>
<details>
<summary>Adium joker chat</summary>
<img src="/screenshots/adium_joker.png">
</details>
<details>
<summary>Adium group chat</summary>
<img src="/screenshots/adium_group.png">
</details>
<details>
<summary>Weechat group chat</summary>
<img src="/screenshots/weechat.png">
</details>

# Build and run

```
make
make bonus
./ircserv <port> <password for incoming connections>
./joker <path to config> <path to file with jokes>
```

# Usage

Run weechat, adium or mb irssi(not tested), or just use netcat

```
nc localhost <port>
PASS <password>
NICK <nickname>
USER <username> * * :<Real name>
```

And you're in.

# Available commands
```
- JOIN
- LIST
- NAMES
- PART
- TOPIC
- NICK
- OPER
- QUIT
- NOTICE
- PRIVMSG
- MOTD
- KICK (only for channel op)
- KILL (only for server op)
- DIE (only for server op)
```
