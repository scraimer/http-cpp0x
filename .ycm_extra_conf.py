import os
import ycm_core

env_home = os.getenv('ENV_HOME','..') 

# These are the compilation flags that will be used in case there's no
# compilation database set (by default, one is not set).
# CHANGE THIS LIST OF FLAGS. YES, THIS IS THE DROID YOU HAVE BEEN LOOKING FOR.
flags = [
		
env_home + '/gmock-1.7.0/gtest/include',
'-isystem',
env_home + '/gmock-1.7.0/include',
'-isystem',
env_home + '/gmock-1.7.0/gtest/include',
'-isystem',
env_home + '/gmock-1.7.0/include',

'-I/home/shalom/Dropbox/dev/x0cpp/http-cppx0/include/',

'-DLinux',
'-Wall',
'-Wshadow',
'-gdwarf-2',
'-Werror',
'-Wstring-compare',
'-Wno-error=string-compare',
'-Wpointer-bool-conversion',
'-Wno-error=pointer-bool-conversion',
'-Wtautological-pointer-compare',
'-Wno-error=tautological-pointer-compare',
'-Wparentheses-equality',
'-Wno-error=parentheses-equality',
'-Winconsistent-missing-override',
'-Werror=inconsistent-missing-override',
'-Wconditional-uninitialized',
'-Werror=conditional-uninitialized',
'-Wshorten-64-to-32',
'-Werror=shorten-64-to-32',
'-Wunused-comparison',
'-Werror=unused-comparison',
'-Wheader-hygiene',
'-Werror=header-hygiene',
'-Wtautological-pointer-compare',
'-Werror=tautological-pointer-compare',
'-Wstring-compare',
'-Werror=string-compare',
'-Wpointer-bool-conversion',
'-Werror=pointer-bool-conversion',
'-Weverything',
'-Wno-reserved-id-macro',
'-Wno-variadic-macros',
'-Wno-gnu-zero-variadic-macro-arguments',
'-Wno-newline-eof',
'-Wno-extra-semi',
'-Wno-unused-macros',
'-Wno-keyword-macro',
'-Wno-documentation',
'-Wno-switch-enum',
'-Wno-c++11-long-long',
'-Wno-c++11-extensions',
'-Wno-unused-parameter',
'-Wno-padded',
'-Wno-packed',
'-Wno-conversion',
'-Wno-global-constructors',
'-Wno-documentation-unknown-command',
'-Wno-unused-exception-parameter',
'-Wno-format-pedantic',
'-Wno-exit-time-destructors',
'-Wno-vla-extension',
'-Wno-vla',
'-Wno-unused-member-function',
'-Wno-c++98-compat-pedantic',
'-Wno-char-subscripts',
'-Wno-unused-variable',
'-Wno-multichar',
'-Wno-undef',
'-Wno-missing-noreturn',
'-Wno-old-style-cast',
'-Wno-deprecated',
'-Wno-covered-switch-default',
'-Wno-float-equal',
'-Wno-cast-align',
'-Wno-non-virtual-dtor',
'-Wno-unused-private-field',
'-Wno-unreachable-code-return',
'-Wno-missing-variable-declarations',
'-Wno-implicit-fallthrough',
'-Wno-unreachable-code-break',
'-Wno-unreachable-code',
'-Wno-sign-conversion',
'-Wno-weak-vtables',
'-Wno-dynamic-class-memaccess',
'-Wno-weak-template-vtables',
'-Wno-used-but-marked-unused',
'-Wno-double-promotion',
'-Wno-double-promotion',
]

# Set this to the absolute path to the folder (NOT the file!) containing the
# compile_commands.json file to use that instead of 'flags'. See here for
# more details: http://clang.llvm.org/docs/JSONCompilationDatabase.html
#
# You can get CMake to generate this file for you by adding:
#   set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )
# to your CMakeLists.txt file.
#
# Most projects will NOT need to set this to anything; you can just change the
# 'flags' list of compilation flags. Notice that YCM itself uses that approach.
compilation_database_folder = ''

if os.path.exists( compilation_database_folder ):
  database = ycm_core.CompilationDatabase( compilation_database_folder )
else:
  database = None

SOURCE_EXTENSIONS = [ '.cpp', '.cxx', '.cc', '.c', '.m', '.mm' ]

def DirectoryOfThisScript():
  return os.path.dirname( os.path.abspath( __file__ ) )


def MakeRelativePathsInFlagsAbsolute( flags, working_directory ):
  if not working_directory:
    return list( flags )
  new_flags = []
  make_next_absolute = False
  path_flags = [ '-isystem', '-I', '-iquote', '--sysroot=' ]
  for flag in flags:
    new_flag = flag

    if make_next_absolute:
      make_next_absolute = False
      if not flag.startswith( '/' ):
        new_flag = os.path.join( working_directory, flag )

    for path_flag in path_flags:
      if flag == path_flag:
        make_next_absolute = True
        break

      if flag.startswith( path_flag ):
        path = flag[ len( path_flag ): ]
        new_flag = path_flag + os.path.join( working_directory, path )
        break

    if new_flag:
      new_flags.append( new_flag )
  return new_flags


def IsHeaderFile( filename ):
  extension = os.path.splitext( filename )[ 1 ]
  return extension in [ '.h', '.hxx', '.hpp', '.hh' ]


def GetCompilationInfoForFile( filename ):
  # The compilation_commands.json file generated by CMake does not have entries
  # for header files. So we do our best by asking the db for flags for a
  # corresponding source file, if any. If one exists, the flags for that file
  # should be good enough.
  if IsHeaderFile( filename ):
    basename = os.path.splitext( filename )[ 0 ]
    for extension in SOURCE_EXTENSIONS:
      replacement_file = basename + extension
      if os.path.exists( replacement_file ):
        compilation_info = database.GetCompilationInfoForFile(
          replacement_file )
        if compilation_info.compiler_flags_:
          return compilation_info
    return None
  return database.GetCompilationInfoForFile( filename )


def FlagsForFile( filename, **kwargs ):
  if database:
    # Bear in mind that compilation_info.compiler_flags_ does NOT return a
    # python list, but a "list-like" StringVec object
    compilation_info = GetCompilationInfoForFile( filename )
    if not compilation_info:
      return None

    final_flags = MakeRelativePathsInFlagsAbsolute(
      compilation_info.compiler_flags_,
      compilation_info.compiler_working_dir_ )

    ## NOTE: This is just for YouCompleteMe; it's highly likely that your project
    ## does NOT need to remove the stdlib flag. DO NOT USE THIS IN YOUR
    ## ycm_extra_conf IF YOU'RE NOT 100% SURE YOU NEED IT.
    #try:
    #  final_flags.remove( '-stdlib=libc++' )
    #except ValueError:
    #  pass
  else:
    relative_to = DirectoryOfThisScript()
    final_flags = MakeRelativePathsInFlagsAbsolute( flags, relative_to )

  return {
    'flags': final_flags,
    'do_cache': True
  }

