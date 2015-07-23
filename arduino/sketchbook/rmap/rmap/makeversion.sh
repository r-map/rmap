echo -n "#define FIRMVERSION \"1.0-" > version.h
svnversion -n . >> version.h
echo -n \" >> version.h
