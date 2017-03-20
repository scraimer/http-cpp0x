import os
import ycm_core

env_home = os.getenv('ENV_HOME','..') 

# These are the compilation flags that will be used in case there's no
# compilation database set (by default, one is not set).
# CHANGE THIS LIST OF FLAGS. YES, THIS IS THE DROID YOU HAVE BEEN LOOKING FOR.
flags = [
		
'-isystem',
'/usr/share/tradigo/clang-3.9/include/c++/v1/',
'-isystem',
'/home/shalom/Dropbox/backups/work/dotfiles/vim/bundle/YouCompleteMe/third_party/ycmd/clang_includes/include',
		
'-isystem',
env_home + '/solclient_Linux26-x86_64_opt/ex',
'-isystem', 
env_home + '/tom/',
'-isystem', 
env_home + '/libssh/src/libssh-0.7.2/include/',
'-isystem',
env_home + '/gmock-1.7.0/gtest/include',
'-isystem',
env_home + '/gmock-1.7.0/include',
'-isystem',
env_home + '/gmock-1.7.0/gtest/include',
'-isystem',
env_home + '/gmock-1.7.0/include',
'-isystem',
env_home + '/rfa8.0.0.L1.linux/Setup/Include',
'-isystem',
env_home + '/rfa8.0.0.L1.linux/Setup/Include/rwf',
'-isystem', 
env_home + '/boost_1_60_0',
'-isystem',
env_home + '/gmock-1.7.0/include',

'-Ilib_utilities',
'-DHAVE_CONFIG_H',
'-DLinux',
'-D REVISION=22131',
'-D LASTMOD_REVISION=22124',
#-D SVN_PATH='"svn://infra.fluent.local/jcfx/tags/v3.6.5"',
#-D UUID='00aad3e5-2610-4058-9e3d-daacbb349d10'
#-D DEFAULT_VER_MAJOR='"3.6"'
'-DPARSE_GENERATION_TIME',
'-DFILL_TIME_FIELD',
'-D_FILE_OFFSET_BITS=64',
'-I/usr/include/libxml2',
'-Ilib_mdp',
'-DUSE_CXX11',
'-DLTC_NO_ASM',
'-DNO_REALTIME_SUPPORT',
'-DLOG_BOOK_RL',
'-DRATEPOOL',
'-DUSE_NEW_NLQ',
'-DPUBLISH_TO_CONTROLLER',
'-DDYN_LOGGER',
'-DREUTERS_SUPPORT',
'-DUSE_SELECT',
'-DORDER_VALIDATION',
'-DRATE_FIELDS_RELEASE',
'-DFASTER_GRID_PARSE',
'-DVERBOSE',
'-DRATE_ACCELERATION',
'-DFIXATION_PROC_MODE_SINGLE',
'-DBUFFER_LOGGER',
'-Idll_fixaton',
'-Iapi/api_impl',
'-Iapi',
'-Ilib_api_client',
'-Ilib_api_server',
'-Ilib_utilities',
'-isystem',
'lib_mdp/otf_api/..',
'-Ilib_mdp/otf_api',
'-Ilib_pugi/..',
'-Ilib_pugi',
'-Ilib_logger',
'-Ilib_trading_system',
'-Ilib_component_manager',
'-Ilib_bridge',
'-Ilib_feed_handlers',
'-Ilib_fix_engine',
'-Ilib_socket_manager',
'-Ilib_models/model_samples',
'-Ilib_models/model_samples/internal_model_samples',
'-Ilib_models/model_aggregator',
'-Ilib_models/model_aggregator/internal_model_samples',
'-Ilib_models',
'-Ilib_models/internal_model_samples',
'-Ilib_fix_sockets',
'-Ilib_fixation',
'-Iunittests',
'-Ilib_remote_access',
'-Ilib_rfa',
'-Ilib_exporter',
'-Ilib_messaging',
'-Itools',
'-Iextendable_sor_flow',
'-Ifluent_stream',
'-Iserver',
'-Iparser',
'-Isbe_generic',
'-Isimulator',
'-Iscripts',
'-Iapi_samples/matrix_demo',
'-Iapi_samples/matrix_maker',
'-Iapi_samples/test_makers',
'-Iapi_samples/test_takers',
'-Iapi_samples/smartrix',
'-Itrade_engine',
'-Imarketmaker',
'-Iflowpro',
'-Ilib',
'-Itools/sqltools',
'-Igtest_unittests',
'-Wall',
'-fnon-call-exceptions',
'-Wshadow',
'-gdwarf-2',
'-std=c++11',
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
'-DRATEBOOK_NOTIFICATIONS',
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

