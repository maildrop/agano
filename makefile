TARGET_VERSION=_WIN32_WINNT_WIN7
CXXOPT=-nologo -EHsc -W4 -WX -O2 -Zi -GS -MDd -Zc:wchar_t -Zc:forScope \
	-DWINVER=$(TARGET_VERSION) -D_WIN32_WINNT=$(TARGET_VERSION) \
	-DWIN32_LEAN_AND_MEAN=1 -DUNICODE=1 -D_UNICODE=1 -DNOMINMAX=1 \
	-DDEBUG=1

.cpp.obj:
	$(CXX) -c $(CXXOPT) $*.cpp
.obj.exe:
	$(CXX) $(CXXOPT) $** -link -DEBUG

VC_PDBFILE=vc140.pdb
all_TARGET=example.exe
example_OBJS=example.obj agano.obj

clean_TARGET=$(all_TARGET) $(all_TARGET:.exe=.pdb) $(all_TARGET:.exe=.ilk) $(example_OBJS)

all: $(all_TARGET)
	@-gtags

example.exe: $(example_OBJS) 

example.obj: example.cpp agano.h whReserved.h whDllFunction.hxx 
agano.obj: agano.cpp agano.h

clean: clean_emacsbackup
	@IF EXIST $(VC_PDBFILE) @del $(VC_PDBFILE)
	@FOR %I IN ( $(clean_TARGET) ) DO @IF EXIST "%~I" del "%~I"

clean_emacsbackup:
	@-FOR /F "usebackq delims=" %I IN (`dir *~ /b 2^>nul`) DO @IF EXIST "%~I" del "%~I"

