
def nint(x):
    """iround(number) -> integer
    Round a number to the nearest integer."""

    y = round(x) - .5
    return int(y) + (y > 0)

def log_dummy( message ):
    """dummy-logger: do nothing"""
    pass
def log_stdout( message ):
    """print message to STDOUT"""
    print message

def log_file( filename ):
    """return a logfunc which logs to a file (in utf-8)"""
    def logfile( message ):
        f = codecs.open( filename, 'a', encoding='utf-8' )
        f.write( message+"\n" )
        f.close()
    return logfile

def log_filedate( filename ):
    """return a logfunc which logs date+message to a file (in utf-8)"""
    def logfile( message ):
        f = codecs.open( filename, 'a', encoding='utf-8' )
        f.write( time.strftime("%Y-%m-%d %H:%M:%S ")+message+"\n" )
        f.close()
    return logfile
