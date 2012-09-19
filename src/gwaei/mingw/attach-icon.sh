#!/bin/sh

WINDRES=i486-mingw32-windres
GPP=i486-mingw32-g++

$WINDRES gwaei.rc -O coff -o gwaei.res
