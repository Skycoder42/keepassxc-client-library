# keepassxc-client-library
A C++ library to access the browser-plugin-API of KeePassXC to retrieve or create entries.

This library is basically a reimplementation of the communication that the [KeePassXC-Browser-Plugin](https://github.com/keepassxreboot/keepassxc-browser) uses to exchange data with [KeePassXC](https://github.com/keepassxreboot/keepassxc). It allows you to connect with a KeePassXC database and use the full browser API in your custom application.

## Features
- A complete, Qt-based implementation of the client side of the [keepassxc-protocol](https://github.com/keepassxreboot/keepassxc-browser/blob/develop/keepassxc-protocol.md)
- Compatible with KeePassXC Version 2.3.0 and above
- End-to-End encryption based on NaCl-Box
- Implements all features of the browser extensions in regards of database access
	- Trigger database unlocks
	- Lock the database
	- Be notified when the database gets un/locked
	- Generate random passwords using KeePassXCs internal generator
	- Create new credentials
	- Retrieve existing credentials based on URLs

## Installation
For now, no prebuilt binaries exist. You have to compile the library yourself. Only linux (and other unixes) are officially supported (for now), but other platforms should work as well, as long as you manually add libsodium as dependency.

### Dependencies
The library only depends on `QtCore` and [`libsodium`](https://download.libsodium.org/doc/). For Unix-Like systems, the libsodium dependency is resolved via pkgconfig. For systems that do not have pkgconfig (like windows) you will have to install that library yourself and manually edit the [src.pro](src/src.pro) file and add the library by hand.

Note: For Unix-Systems, the library comes with an automatically generated pkgconfig file called `libkpxcclient.pc` for easy integration of the library into your project. For qmake based projects, this can be done using:

```.pro
CONFIG += link_pkgconfig
PKGCONFIG += libkpxcclient
```

### Compilation
The library uses `qmake` as build systems. Simply run `qmake [build-options]`, followed by `make` and `make install`. The supported build options are:

- `PREFIX=...`: A custom installation prefix, specifying where to install the library to. By default, the library is installed into your Qt-Installation. You can also fine-tune sub-paths. See [install.pri](install.pri) for all possible values.
- `CONFIG+=install_private_headers`: Install all private headers in addition to the public headers in a subdirectory called `private`
- `CONFIG+=install_demo`: Install the demo-binary as well. By default, only the library itself is installed.

## Usage
The primary class of the library is `KPXCClient::Client`. It manages the connection to KeePassXC and provides all the possible operations and events in form of signals and slots. The use the library, you have to initialize it once in your main:

```.cpp
KPXCClient::init();
```

After that, you can use the client itself to connect to a database:

```.cpp
KPXCClient::Client client;
QObject::connect(&client, &KPXCClient::Client::databaseOpened, [&](QByteArray dbHash) {
	qDebug() << "Connected to database:" << dbHash.toHex();
	// perform whatever operations you need to perform
});
client.connectToKeePass();
```

The behaviour of the client is manly dictated by a few properties. Most notably the `KPXCClient::Client::options` property. Check out the corresponding header files to get a grasp of all its capabilities. A formal API-documentation is planned, but was not created yet.

### Demo Application
The library comes with a small demo that presents all of the supported operations as a short linear run on a KeePassXC database. The full code can be found in [clidemo/main.cpp](clidemo/main.cpp). The demo is automatically built with the library, but not installed by default. Please note that it will *modify* the selected database, so it is recommended to run the demo on a test database.

## References
- [KeePassXC](https://github.com/keepassxreboot/keepassxc)
- [KeePassXC-Browser-Plugin](https://github.com/keepassxreboot/keepassxc-browser)
- [keepassxc-protocol](https://github.com/keepassxreboot/keepassxc-browser/blob/develop/keepassxc-protocol.md)
- [libsodium](https://download.libsodium.org/doc/)
