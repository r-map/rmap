# https://gist.github.com/rochacbruno/5506616

# Orignal version taken from http://www.djangosnippets.org/snippets/186/
# Original author: udfalkso
# Modified by: Shwagroo Team and Gun.io

# How to use hotshot profiling
# http://127.0.0.1:8000/anything/?prof

# How to use CProfile
# http://127.0.0.1:8000/anything/?cprof
# http://127.0.0.1:8000/anything/?cprof=cumulative
# cprof="cprofile_collums_to_sort"

import sys
import os
import re
import tempfile

from django.conf import settings

import cProfile
import pstats

words_re = re.compile(r'\s+')

group_prefix_re = [
    re.compile("^.*/django/[^/]+"),
    re.compile("^(.*)/[^/]+$"),  # extract module path
    re.compile(".*"),  # catch strange entries
]


#################################
##  hotshot only in python 2   ##
##  this is for python 2       ##
#################################
#
#import hotshot
#import hotshot.stats
#import StringIO
#
#class ProfileMiddleware(object):
#    """
#    Displays hotshot profiling for any view.
#    http://yoursite.com/yourview/?prof
#
#    Add the "prof" key to query string by appending ?prof (or &prof=)
#    and you'll see the profiling results in your browser.
#    It's set up to only be available in django's debug mode, is available for superuser otherwise,
#    but you really shouldn't add this middleware to any production configuration.
#
#    WARNING: It uses hotshot profiler which is not thread safe.
#    """
#    def process_request(self, request):
#        if (settings.DEBUG or request.user.is_superuser) and 'prof' in request.GET:
#            self.tmpfile = tempfile.mktemp()
#            self.prof = hotshot.Profile(self.tmpfile)
#
#    def process_view(self, request, callback, callback_args, callback_kwargs):
#        if (settings.DEBUG or request.user.is_superuser) and 'prof' in request.GET:
#            return self.prof.runcall(callback, request, *callback_args, **callback_kwargs)
#
#    def get_group(self, file):
#        for g in group_prefix_re:
#            name = g.findall(file)
#            if name:
#                return name[0]
#
#    def get_summary(self, results_dict, sum):
#        list = [(item[1], item[0]) for item in results_dict.items()]
#        list.sort(reverse=True)
#        list = list[:40]
#
#        res = "      tottime\n"
#        for item in list:
#            res += "%4.1f%% %7.3f %s\n" % (100*item[0]/sum if sum else 0, item[0], item[1])
#
#        return res
#
#    def summary_for_files(self, stats_str):
#        stats_str = stats_str.split("\n")[5:]
#
#        mystats = {}
#        mygroups = {}
#
#        sum = 0
#
#        for s in stats_str:
#            fields = words_re.split(s)
#            if len(fields) == 7:
#                time = float(fields[2])
#                sum += time
#                file = fields[6].split(":")[0]
#
#                if not file in mystats:
#                    mystats[file] = 0
#                mystats[file] += time
#
#                group = self.get_group(file)
#                if not group in mygroups:
#                    mygroups[group] = 0
#                mygroups[group] += time
#
#        return "<pre>" + \
#               " ---- By file ----\n\n" + self.get_summary(mystats, sum) + "\n" + \
#               " ---- By group ---\n\n" + self.get_summary(mygroups, sum) + \
#               "</pre>"
#
#    def process_response(self, request, response):
#        if (settings.DEBUG or request.user.is_superuser) and 'prof' in request.GET:
#            self.prof.close()
#
#            out = StringIO.StringIO()
#            old_stdout = sys.stdout
#            sys.stdout = out
#
#            stats = hotshot.stats.load(self.tmpfile)
#            stats.sort_stats('time', 'calls')
#            stats.print_stats()
#
#            sys.stdout = old_stdout
#            stats_str = out.getvalue()
#
#            if response and response.content and stats_str:
#                response.content = "<pre>" + stats_str + "</pre>"
#
#            response.content = "\n".join(response.content.split("\n")[:40])
#
#            response.content += self.summary_for_files(stats_str)
#
#            os.unlink(self.tmpfile)
#
#        return response


#################################
##  this is for python 3       ##
#################################

#import cStringIO
from io import StringIO

class ProfileMiddleware(object):
    def process_view(self, request, callback, callback_args, callback_kwargs):
        if settings.DEBUG and 'cprof' in request.GET:
            self.profiler = cProfile.Profile()
            args = (request,) + callback_args
            return self.profiler.runcall(callback, *args, **callback_kwargs)

    def process_response(self, request, response):
        if settings.DEBUG and 'cprof' in request.GET:
            (fd, self.profiler_file) = tempfile.mkstemp()
            self.profiler.dump_stats(self.profiler_file)
            out = StringIO()
            stats = pstats.Stats(self.profiler_file, stream=out)
            stats.strip_dirs()          # Must happen prior to sort_stats
            if request.GET['cprof']:
                stats.sort_stats(request.GET['cprof'])
            stats.print_stats()
            os.unlink(self.profiler_file)
            response.content = '<pre>%s</pre>' % out.getvalue()
        return response
