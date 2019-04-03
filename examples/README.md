# Build Examples

Requires `Qt` installation with `QMake`. To generate the respective project files run the commands below.

## In Linux

```cmake
qmake -r examples.pro
```

## In Windows

Make sure to use Qt's command prompt which is tipically installed with Qt (`qtenv2.bat`).

```cmake
qmake -r -tp vc examples.pro
```