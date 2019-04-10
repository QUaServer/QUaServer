# QUaServer

This is a [Qt](https://www.qt.io/) based library that provides a C++ **wrapper** for the [open62541](https://open62541.org/) library, and **abstraction** for the OPC UA Server API.

By *abstraction* it is meant that some of flexibility provided by the original *open62541* server API is sacrificed for ease of use. If more flexibility is required than what *QUaServer* provides, it is highly recommended to use the original *open62541* instead.

The main goal of this library is to provide an object-oriented API that allows quick prototyping for OPC UA servers without having to spend much time in creating complex address space structures.

*QUaServer* is still work in progress, test properly and use precaution before using in production. Please report any issues you encounter in this repository providing a minimum working code example that replicates the issue and a thorough description.

```
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```

To test a *QUaServer* based application it is recommended to use the [UA Expert](https://www.unified-automation.com/downloads/opc-ua-clients.html) OPC UA Client.

---

## Include

This library requires at least `Qt 5.7` or higher and `C++ 11`.

To use *QUaServer*, first a copy of the *open62541* shared library is needed. An amalgamation of the latest compatible *open62541* version is included in this repository for convenience in [./src/amalgamation](./src/amalgamation). 

The amalgamation included in this repository was created using the following *CMake* command:

```bash
cd open62541.git
mkdir build; cd build;
cmake -DUA_ENABLE_AMALGAMATION=ON .. -G "Visual Studio 15 2017 Win64"
```

The `open62541.pro` Qt project can be used to build the included amalgamation into the required shared library using the command:

```bash
cd ./src/amalgamation
# Linux
qmake open62541.pro
make
# Windows
qmake -tp vc open62541.pro
```

If it is desired to use a more recent version of *open62541*, it is possible to build the amalgamation using the soure code in the [open62541 git repository](https://github.com/open62541/open62541) and then replace [./src/amalgamation/open62541.h](./src/amalgamation/open62541.h) and [./src/amalgamation/open62541.c](./src/amalgamation/open62541.c). Though compatibility of *QUaServer* with the latest version of *open62541* is not guaranteed.

Finally to include *QUaServer* in your project, just include [./src/wrapper/quaserver.pri](./src/wrapper/quaserver.pri) into your Qt project file (`*.pro` file). For example:

```cmake
QT += core
QT -= gui

CONFIG += c++11

TARGET = my_project
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$PWD/

SOURCES += main.cpp

include($$PWD/../../src/wrapper/quaserver.pri)
```

---

## Basics

To start using *QUaServer* it is necessary to include the `QUaServer` header as follows:

```c++
#include <QUaServer>
```

To create a server simple create an `QUaServer` instance and call the `start()` method:

```c++
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	// create server
	QUaServer server;
	// start server
	server.start();

    return a.exec(); 
}
```

Note it is necessary to create a `QCoreApplication` and execute it, because `QUaServer` makes use of [Qt's event loop](https://wiki.qt.io/Threads_Events_QObjects).

By default the *QUaServer* listens on port **4840** which is the [IANA assigned port](https://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.xhtml?search=4840) for OPC UA applications. To change the listening port, simply pass it as the first argument of the *QUaServer* constructor:

```c++
QUaServer server(8080);
```

To start creating OPC *Objects* and *Variables* it is necessary to get the *Objects Folder* of the server and start adding instances to it:

```c++
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	QUaServer server;

    // get objects folder
	QUaFolderObject * objsFolder = server.objectsFolder();

	// add some instances to the objects folder
	QUaBaseDataVariable * varBaseData = objsFolder->addBaseDataVariable();
	QUaProperty         * varProp     = objsFolder->addProperty();
	QUaBaseObject       * objBase     = objsFolder->addBaseObject();
	QUaFolderObject     * objFolder   = objsFolder->addFolderObject();

	server.start();

    return a.exec(); 
}
```

Once connected to the server, the address space should look something like this:

<p align="center">
  <img src="./res/img/01_basics_01.jpg">
</p>