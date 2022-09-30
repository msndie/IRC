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


Servers and clients send each other messages which may or may not
generate a reply.  If the message contains a valid command, as
described in later sections, the client should expect a reply as
specified, but it is not advised to wait forever for the reply; client
to server and server to server communication is essentially
asynchronous in nature.

Each IRC message may consist of up to three main parts: the prefix
(optional), the command, and the command parameters (of which there
may be up to 15).  The prefix, command, and all parameters are
separated by one (or more) ASCII space character(s) (0x20).

The presence of a prefix is indicated with a single leading ASCII
colon character (':', 0x3b), which must be the first character of the
message itself.  There must be no gap (whitespace) between the colon
and the prefix.  The prefix is used by servers to indicate the true 
origin of the message.  If the prefix is missing from the message, it
is assumed to have originated from the connection from which it was
received.  Clients should not use prefix when sending a message from
themselves; if they use a prefix, the only valid prefix is the
registered nickname associated with the client.  If the source
identified by the prefix cannot be found from the server's internal
database, or if the source is registered from a different link than
from which the message arrived, the server must ignore the message
silently.

The command must either be a valid IRC command or a three (3) digit
number represented in ASCII text.

IRC messages are always lines of characters terminated with a CR-LF
(Carriage Return - Line Feed) pair, and these messages shall not
exceed 512 characters in length, counting all characters including
the trailing CR-LF. Thus, there are 510 characters maximum allowed
for the command and its parameters.  There is no provision for
continuation message lines.  See section 7 for more details about
current implementations.