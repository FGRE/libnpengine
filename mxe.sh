#!/bin/bash
export PATH=$MXE/usr/bin
i686-pc-mingw32-g++ -DSFML_STATIC -std=gnu++11 src/*.cpp -I include/ \
-o libnpengine.dll -I ../libnpa/include/ -lboost_system-mt -DBOOST_THREAD_USE_LIB \
-L../libnpa/ -lnpa -lboost_thread_win32-mt -lboost_chrono-mt \
-mwindows -L/home/mislav/Devel/mxe/usr/i686-pc-mingw32/lib -lsfeMovie -lsfml-audio-s \
-lsfml-network-s -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -ljpeg -lavfilter \
-lGLEW -lopengl32 -lOpenAL32 -luuid -lsndfile -lFLAC -lglu32 -lpostproc -lswresample \
-lswscale -lavformat -lavcodec -lavicap32 -lmingw32  -luser32 -ldxguid \
-lxvidcore -lx264 -lvpx -lpthread -lvorbisenc -lvorbis -lvo-amrwbenc -lvo-aacenc \
-ltheoraenc -ltheoradec -logg -lspeex -lopus -lopencore-amrwb -lopencore-amrnb \
-lmp3lame -lass -lharfbuzz -lusp10 -lfribidi -lcairo -lmsimg32 -lgdi32 -lgobject-2.0 \
-lffi -lpixman-1 -lfontconfig -lexpat -lfreetype -lpng16 -lglib-2.0 -lws2_32 -lole32 \
-lwinmm -lshlwapi -lintl -liconv -lpcre -lavifil32 -lbz2 -lz -lpsapi -ladvapi32 \
-lshell32 -lavutil -lm -mwindows -shared
