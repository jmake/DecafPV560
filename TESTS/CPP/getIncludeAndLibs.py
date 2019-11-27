import os
#-------------------------------------------------------------------------------------#
import argparse
parser = argparse.ArgumentParser()
parser.add_argument('-F', action='store', dest='CMAKE_PROJECT',
                    default=[], type=str, nargs='+',
                    help='USAGE: -F "CMAKE PROJECT NAME" ',
                    )

#(options, args) = parser.parse_args()
options = parser.parse_args()
print(  ) 
if( not len(options.CMAKE_PROJECT) ): 
  parser.print_help()
  exit(1)

#-------------------------------------------------------------------------------------#
GetIncludeAndLibs = """ 
#GetIncludeAndLibs()
#{
  CATALYST_CPPFLAGS=`grep CXX_DEFINES CMakeFiles/CASE.dir/flags.make | sed -e 's/^[^=]*=[\ ]*//'`
  CATALYST_INCLUDES=`grep CXX_INCLUDES CMakeFiles/CASE.dir/flags.make | sed -e 's/^[^=]*=[\ ]*//'`
  CATALYST_CPPFLAGS="${CATALYST_CPPFLAGS} ${CATALYST_INCLUDES}"
  CATALYST_CXXFLAGS=`grep CXX_FLAGS CMakeFiles/CASE.dir/flags.make | sed -e 's/^[^=]*=[\ ]*//'`
  cs_link_opts=`sed -e 's/^.*CASE//' CMakeFiles/CASE.dir/link.txt`
  for a in $cs_link_opts; do
      case "{$a}" in
      -l*) CATALYST_LIBS="${CATALYST_LIBS} ${a}"
           ;;
      *)   if test -f "$a" ; then
             CATALYST_LIBS="${CATALYST_LIBS} -Wl,${a}"
           else
             CATALYST_LDFLAGS="${CATALYST_LDFLAGS} ${a}"
           fi
           ;;
      esac
  done

  echo $CATALYST_INCLUDES               > includes.in
  echo $CATALYST_LIBS $CATALYST_LDFLAGS > libs.in
#} 
""" 
#-------------------------------------------------------------------------------------#

##
GetIncludeAndLibs = GetIncludeAndLibs.replace("CASE", options.CMAKE_PROJECT[0]) 
os.system(GetIncludeAndLibs) 

## CATALYST_LIBS 
Fin  = open("libs.in", "r")
Libs = Fin.read() 
Fin.close() 
os.system("mv libs.in libs.old")

Libs = Libs.strip().split()
Libs = [lib for lib in Libs if 'Wl' in lib]
Libs = [lib.split(",")[-1] for lib in Libs if 'Wl' in lib]
Libs = ["CATALYST_LIBS+=%s\n"%lib for lib in Libs] 
Fout = open("libs.in","w")
Fout.write( "".join(Libs) ) 
Fout.close()

## CATALYST_INCS
Fin  = open("includes.in", "r")
Libs = Fin.read()
Fin.close()
os.system("mv includes.in includes.old")

Libs = Libs.strip().split()
Libs = ["CATALYST_INCS+=%s\n"%lib for lib in Libs]
Fout = open("includes.in","w")
Fout.write( "".join(Libs) )
Fout.close()



