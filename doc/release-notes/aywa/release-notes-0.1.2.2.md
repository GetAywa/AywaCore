0.1.2.2 Release notes
====================


Aywa Core version 0.1.2.2 is now available from:

  http://www.getaywa.org/downloads

Please report bugs using the issue tracker at github:

  https://github.com/getayaywa/aywacore/issues



# Upgrading and downgrading

## How to Upgrade

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions), then run the
installer (on Windows) or just copy over /Applications/Aywa-Qt (on Mac) or
aywad/aywa-qt (on Linux).

# Notable changes

## Command-line options

New: `gethashrate`

Experimental: ` `

See `Help -> Command-line options` in Qt wallet or `aywad --help` for more info.

## Lots of backports, refactoring and bug fixes

We backported some performance improvements from Bitcoin Core / Dash and aligned our codebase with their source a little bit better. We still do not have all the improvements so this work is going to be continued in next releases.

A lot of refactoring and other fixes should make code more reliable and easier to review now.

# 0.1.2.2 Change log

Detailed [change log](https://github.com/getaywa/aywacore/compare/v0.1.2.1...aywacore:v0.1.2.2) below.