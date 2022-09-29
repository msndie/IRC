# IRC

https://beej.us/guide/bgnet/html/index-wide.html

nickname length 9 characters ( letter / special ) *8( letter / digit / special / "-" )

special    =  %x5B-60 / %x7B-7D
; "[", "]", "\", "`", "_", "^", "{", "|", "}"

```
Channels names are strings (beginning with a '&', '#', '+' or '!'
character) of length up to fifty (50) characters.  Apart from the
requirement that the first character is either '&', '#', '+' or '!',
the only restriction on a channel name is that it SHALL NOT contain
any spaces (' '), a control G (^G or ASCII 7), a comma (',').  Space
is used as parameter separator and command is used as a list item
separator by the protocol).  A colon (':') can also be used as a
delimiter for the channel mask.  Channel names are case insensitive.
```

The commands which may only be used by channel operators are:

KICK    - Eject a client from the channel
MODE    - Change the channel's mode
INVITE  - Invite a client to an invite-only channel (mode +i)
TOPIC   - Change the channel topic in a mode +t channel

A channel operator is identified by the '@' symbol next to their
nickname whenever it is associated with a channel (ie replies to the
NAMES, WHO and WHOIS commands).