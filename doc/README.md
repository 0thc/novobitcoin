Novo Bitcoin
=====================

Setup
---------------------
Novo Bitcoin is an implementation of a node for the Novo Bitcoin network and is one of the pieces of software that provide
the backbone of the network. It downloads and stores the entire history of Novo Bitcoin transactions; depending on the speed
of your computer and network connection, the synchronization process can take anywhere from a few minutes to a hour or more.

To download Novo Bitcoin, visit [novobitcoin.org](https://novobitcoin.org/).

Running
---------------------
Novo Bitcoin is only supported on the Linux and docker platforms at this time.

To run Novo Bitcoin on Linux:

* unpack the files into a directory
* run `bin/novobitcoind`

### Need Help?

* Log an issue on [GitHub] (https://github.com/novobitcoin/novobitcoin/issues)
* Ask for help on the [Novo Bitcoin Forum](https://forum.novobitcoin.org/).
* Consult [Novo Bitcoin Wiki](https://wiki.novobitcoin.org/) for information about Bitcoin protocol.

Building
---------------------
The following are developer notes on how to build Bitcoin. They are not complete guides, but include notes on the
necessary libraries, compile flags, etc.

- [Unix Build Notes](build-unix.md)
- [Gitian Building Guide](gitian-building.md)

Development
---------------------
The Novo Bitcoin repo's [root README](/README.md) contains relevant information on the development process and automated
testing.

- [Developer Notes](developer-notes.md)
- [Release Notes](release-notes.md)
- [Unauthenticated REST Interface](REST-interface.md)
- [Shared Libraries](shared-libraries.md)
- [BIPS](bips.md)
- [Benchmarking](benchmarking.md)

### Miscellaneous
- [Assets Attribution](assets-attribution.md)
- [Files](files.md)
- [Fuzz-testing](fuzzing.md)
- [Reduce Traffic](reduce-traffic.md)
- [Init Scripts (systemd/upstart/openrc)](init.md)
- [ZMQ](zmq.md)

License
---------------------
Distribution is done under the [MIT License](/LICENSE). This product includes software developed by the OpenSSL
Project for use in the [OpenSSL Toolkit](https://www.openssl.org/), cryptographic software written by Eric Young
([eay@cryptsoft.com](mailto:eay@cryptsoft.com)), and UPnP software written by Thomas Bernard.
