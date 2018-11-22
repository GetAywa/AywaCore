# AywaCore
Aywa is Decentralized Communication and 🎁 Rewards Platform with 🖥 CPU-only mining algo, based on hash algo YesPower, Groestl and Keccak🔐. Aywa uses new Time Gravity Wave algo for difficulty 📊 adjustment and different cost of Masternodes.

Enjoy Aywa built-in IdeaMining. Many proposals already sends rewards🎁🎁🎁 their owners. Needs some AYWA to submit your proposal? No problem! Just press "Start Mining" 🔲 button at your AywaCore Overview Page.

More information is on our page: www.getaywa.org.

If required information was not found you can ask questions at the AywaCore built-in messanger. 

## [AywaCore Download](https://github.com/GetAywa/AywaCore/releases)

Look at some AywaCore features:

Overview Page

![](http://getaywa.org/wp-content/uploads/2018/09/preview_video.png)

Proposal Creation
￼
![www.getaywa.org](http://getaywa.org/wp-content/uploads/2018/10/ProposalCreation.png)

Voting Process

![](http://getaywa.org/wp-content/uploads/2018/10/proposal_page.png)￼

Messages and Public Channels

![](http://getaywa.org/wp-content/uploads/2018/10/messagges_page.png)
![](http://getaywa.org/wp-content/uploads/2018/10/smsg3.png)

Reward for Proposal
￼
![](http://getaywa.org/wp-content/uploads/2018/10/reward_for_proposal.png)

License
-------

Aywa Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

The `master` branch is meant to be stable. Development is normally done in separate branches.
[Tags](https://github.com/getaywa/aywacore/tags) are created to indicate new official,
stable release versions of Aywa Core.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md).

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](/doc/unit-tests.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`

There are also [regression and integration tests](/qa) of the RPC interface, written
in Python, that are run automatically on the build server.
These tests can be run (if the [test dependencies](/qa) are installed) with: `qa/pull-tester/rpc-tests.py`

The Travis CI system makes sure that every pull request is built for Windows
and Linux, OS X, and that unit and sanity tests are automatically run.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.

Translations
------------

Changes to translations as well as new translations can be submitted to
[Aywa Core's Issues page](https://bitbucket.org/CryptoDev_Space/aywacore/issues/).

Translations are periodically pulled from Transifex and merged into the git repository. See the
[translation process](doc/translation_process.md) for details on how this works.

**Important**: We do not accept translation changes as GitHub pull requests because the next
pull from Transifex would automatically overwrite them again.
