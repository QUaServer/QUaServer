# test if windows
win32 {
  # test if already exists
  VCXPROJ_USER_FILE = "$${_PRO_FILE_PWD_}/$${TARGET}.vcxproj.user"
  !exists( $${VCXPROJ_USER_FILE}) {
    # generate file contents
    TEMPNAME         = $${QMAKE_QMAKE} # contains full dir of qmake used
    DEPENDS_DIR_REL  = $$dirname(TEMPNAME)
    DEPENDS_DIR_DBG  = $$dirname(TEMPNAME)
    # vcxproj.user template
    VCXPROJ_USER = "<?xml version=\"1.0\" encoding=\"utf-8\"?>$$escape_expand(\\n)\
    <Project xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">$$escape_expand(\\n)\
      <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\">$$escape_expand(\\n)\
        <LocalDebuggerEnvironment>PATH=$${DEPENDS_DIR_DBG};%PATH%</LocalDebuggerEnvironment>$$escape_expand(\\n)\
        <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>$$escape_expand(\\n)\
      </PropertyGroup>$$escape_expand(\\n)\
      <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|Win32'\">$$escape_expand(\\n)\
        <LocalDebuggerEnvironment>PATH=$${DEPENDS_DIR_REL};%PATH%</LocalDebuggerEnvironment>$$escape_expand(\\n)\
        <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>$$escape_expand(\\n)\
      </PropertyGroup>$$escape_expand(\\n)\
      <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">$$escape_expand(\\n)\
        <LocalDebuggerEnvironment>PATH=$${DEPENDS_DIR_DBG};%PATH%</LocalDebuggerEnvironment>$$escape_expand(\\n)\
        <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>$$escape_expand(\\n)\
      </PropertyGroup>$$escape_expand(\\n)\
      <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|x64'\">$$escape_expand(\\n)\
        <LocalDebuggerEnvironment>PATH=$${DEPENDS_DIR_REL};%PATH%</LocalDebuggerEnvironment>$$escape_expand(\\n)\
        <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>$$escape_expand(\\n)\
      </PropertyGroup>$$escape_expand(\\n)\
    </Project>$$escape_expand(\\n)\
    "
    # write file
    write_file($${VCXPROJ_USER_FILE}, VCXPROJ_USER)  
  }
}