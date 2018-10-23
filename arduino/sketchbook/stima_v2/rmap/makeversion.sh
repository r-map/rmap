echo -n "#define FIRMVERSION \"1.0-" > version.h
#svnversion -n . >> version.h
GITVERSION=`git log --oneline | wc -l`
echo -n $GITVERSION\" >> version.h
