# Zephyrus G14 (2021) N-Key Rollover Fix for Linux

## The Problem

I have a Zephyrus G14 from 2021, and I run Linux on it.  In 2025, a game called PROJEKT GODHAND released, which is a very fun and unhinged
rhythm game that requires a *minimum* of 8 keys to play in its "Hellbent" mode. While playing it, I quickly realised there was a serious
issue with my laptop's keyboard: its NKRO feature doesn't actually work properly on Linux.

If I hold down 7 or more keys on my keyboard, then release one of the first 6 keys I pressed, a release event is incorrectly triggered for
the 7th key even though I'm still physically holding it down. If I'm holding more than 7, subsequent releases of the first 6 keys will
continue to trigger incorrect releases for keys 8, 9, and so on. This breaks 7-holds and 8-holds as soon as additional notes are added to
the pattern.

Of course, I could play PROJEKT GODHAND in Glorious mode, designed for keyboards which only support 6KRO. However, my laptop advertises NKRO
for its keyboard, and I'd very much like that feature to work properly when I'm playing rhythm games.

Fortunately, I made two observations about these events:

1. The incorrect event is always triggered *after* the correct event
2. Both events have the exact same timestamp, down to the microsecond

So after much reading about what I can do about this, I wrote a program that uses libevdev to filter out those unwanted release events,
then redirect everything else to a virtual input device. When a release event is suppressed, a press event for that key is injected back
into the original input device, allowing the correct release event to be triggered when the key is physically released. As far as I can
tell, this fixes my problem perfectly.

## Who is this for?

I made it for myself, but it can also be used by anyone who happens to have the same combination of hardware, operating system, and taste
in video games as me, on the off chance that the aforementioned subset of humanity somehow consists of more than one person.

## Compiling

For Debian-based distros, run the following commands:

`sudo apt install libevdev-dev build-essential`

``gcc main.c `pkg-config --cflags --libs libevdev` -o g14-nkro``

If you're not using a Debian-based distro, please excuse my lack of familiarity with your package manager and repositories.

## Running

To run, put the `g14-nkro` executable somewhere on your PATH and run:

`g14-nkro /dev/input/event_`

Replace the underscore at the end with the actual ID of your keyboard. You can find which number to use by running `evtest`.

You may need to give it the execute permission if you downloaded it from the Releases page.

Confirm it's working by pressing and holding at least 7 keys, then releasing one of the first 6 you pressed. A message should be printed
to standard output regardless of whether the program is in focus.

## Notes

This program is actually a very quick and desperate hack that I made to serve a single use case, and I have NOT tested it for anything else.
In the interest of Not Breaking Things, please don't use this program outside of its use case, and don't leave it running when you don't
need it.

I still have no idea what causes this weird keyboard behaviour. There's probably a better way to fix it in the kernel, but I have zero
knowledge of kernel stuff.

I'm pleased to say that **ZERO** generative AI was used in the creation of this program and readme!
