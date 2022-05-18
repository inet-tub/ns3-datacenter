# Copyright (C) 2006-2009 Gustavo J. A. M. Carneiro  <gjcarneiro@gmail.com>

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

"""
A bzr plugin that allows bzr log to output in GNU ChangeLog format.

Installation:
  Copy this file to ~/.bazaar/plugins/

Usage:
  bzr log -v --log-format 'gnu'

  or,

  BZR_GNULOG_SPLIT_ON_BLANK_LINES=0 bzr log -v --log-format 'gnu'

  By default the log formatter recognizes paragraph boundaries as
  empty lines (like in LaTeX). The environment variable
  BZR_GNULOG_SPLIT_ON_BLANK_LINES=0 switches it to split paragraphs by
  the newline character instead.

Changes:
  - 2009-07-12: Missing supports_merge_revisions added, to also show
                merged revisions from non-main branches
  - 2009-06-24: handle get_apparent_author deprecation (based on patch by Zdenek Hatas)
  - 2009-04-04: add option to recognize newlines as paragraph boundaries too
  - 2009-04-04: recognize blank lines as paragraph boundaries, and don't mix paragraphs
  - 2009-04-04: output text in default system encoding, not ascii
  - 2008-12-08: include information of fixed bugs (bzr commit --fixes)
"""

import time
import textwrap
from StringIO import StringIO # note: we can't use cStringIO; it does not handle unicode well.
import sys
import os

import bzrlib.log
#from bzrlib.osutils import format_date

SPLIT_ON_BLANK_LINES = bool(int(os.environ.get('BZR_GNULOG_SPLIT_ON_BLANK_LINES', "1")))

class GnuFormatter(bzrlib.log.LogFormatter):

    def __init__(self, *args, **kwargs):
        bzrlib.log.LogFormatter.__init__(self, *args, **kwargs)
        self._changes_buffer = StringIO()
        self._date_line = None
        self.supports_tags = True
        self.supports_delta = True
        self.supports_merge_revisions = True

    def log_revision(self, lr):
        self.show(lr.revno, lr.rev, lr.delta, lr.tags)
    
    def show(self, revno, rev, delta, tags=None):
        to_file = self.to_file
        if tags is not None:
            self._flush()
            print >> to_file, u"=== %s ===" % ', '.join(tags)
        date_str = time.strftime("%Y-%m-%d", time.localtime(rev.timestamp))
        if hasattr(rev, "get_apparent_authors"):
            author = "\n\t    ".join(rev.get_apparent_authors())
        else:
            author = rev.get_apparent_author().strip()
        date_line = date_str + "  " + author
        if date_line != self._date_line:
            self._flush()
            self._date_line = date_line
        self._show_changes(revno, rev, delta)

    def _flush(self):
        if self._date_line is None or not self._changes_buffer.getvalue():
            return
        to_file = self.to_file
        print >> to_file, self._date_line
        print >> to_file
        s = self._changes_buffer.getvalue()
        to_file.write(s)
        self._changes_buffer.truncate(0)

    def begin_log(self):
        pass

    def end_log(self):
        self._flush()

    def __del__(self): # this is evil, I need bzr to call a method on me when it's finished..
        self._flush()

    def _split_message(self, message):
        """Split message into a shortlog plus list of paragraphs.
        Paragraphs are identified by splitting the message where blank
        lines appear, or simply where a newline appears, depending on
        the SPLIT_ON_BLANK_LINES global variable.

        returns:  (shortlog, paragraph_list)        
        """
        lines = message.split('\n')
        if lines:
            # the first line is always the shortlog
            shortlog = lines.pop(0)
            # the second line may be empty to separate the shortlog from the detailed message
            if lines and not lines[0]:
                lines.pop(0)
        else:
            shortlog = u'(no commit message)'
        if SPLIT_ON_BLANK_LINES:
            paragraphs = []
            current_paragraph = []
            for line in lines:
                if not line: # empty line marks the end of a paragraph
                    paragraphs.append('\n'.join(current_paragraph))
                    current_paragraph = []
                else:
                    current_paragraph.append(line)
            if current_paragraph:
                paragraphs.append('\n'.join(current_paragraph))
            return shortlog, paragraphs
        else:
            return shortlog, lines

    def _show_changes(self, revno, rev, delta):
        to_file = self._changes_buffer
        output_standalone_message = True
        shortlog, paragraphs = self._split_message(rev.message)
        shortlog = u"[%s] %s" % (revno, shortlog)
        bugs_text = rev.properties.get('bugs', '')
        if bugs_text:
            if shortlog[-1] in '.;':
                shortlog = shortlog[:-1]
            shortlog = u"%s; %s" % (shortlog, bugs_text)
        
        if delta is not None:
            ## special case when only text modifications exist
            if delta.modified and not (delta.added or delta.removed
                                       or delta.renamed):
                for num, (path, id, kind, text_modified, meta_modified) \
                        in enumerate(delta.modified):
                    if num == len(delta.modified) - 1:
                        for line in textwrap.wrap(
                                u"%s: %s" % (path, shortlog),
                                width=70,
                                initial_indent="* ",
                                subsequent_indent=""):
                            print >> to_file, u"\t" + line
                    else:
                        print >> to_file, u"\t* %s," % (path,)
                if paragraphs:
                    print >> to_file
                for parI, par in enumerate(paragraphs):
                    if par:
                        for l2 in textwrap.wrap(par, 70):
                            print >> to_file,  u'\t' + l2
                    else:
                        if parI < (len(paragraphs)-1):
                            print >> to_file
                    if SPLIT_ON_BLANK_LINES:
                        print >> to_file
                if not paragraphs or not SPLIT_ON_BLANK_LINES:
                    print >> to_file
                output_standalone_message = False

            else:

                self._flush()
                
                if delta.added:
                    for num, (path, id, kind) in enumerate(delta.added):
                        if num == len(delta.added) - 1:
                            print >> to_file, u"\t* %s: Added." % (path,)
                            print >> to_file
                        else:
                            print >> to_file, u"\t* %s," % (path,)

                if delta.removed:
                    for num, (path, id, kind) in enumerate(delta.removed):
                        if num == len(delta.removed) - 1:
                            print >> to_file, u"\t* %s: Removed." % (path,)
                            print >> to_file
                        else:
                            print >> to_file, u"\t* %s," % (path,)

                if delta.modified:
                    for num, (path, id, kind, text_modified, meta_modified) \
                            in enumerate(delta.modified):
                        if num == len(delta.modified) - 1:
                            print >> to_file, u"\t* %s: Modified." % (path,)
                            print >> to_file
                        else:
                            print >> to_file, u"\t* %s," % (path,)

                if delta.renamed:
                    for (oldpath, newpath, id, kind,
                         text_modified, meta_modified) in delta.renamed:
                        if text_modified:
                            line = (u"%s: Renamed to %s and modified."\
                                    % (oldpath, newpath))
                        else:
                            line = u"%s: Renamed to %s." % (oldpath, newpath)
                        for l1 in textwrap.wrap(line, 70,
                                                initial_indent="* ",
                                                subsequent_indent="  "):
                            print >> to_file, u"\t" + l1
                    print >> to_file

        if output_standalone_message:
            ## The log message, on a line by itself
            paragraphs.insert(0, shortlog)
            for parI, par in enumerate(paragraphs):
                if par:
                    for l2 in textwrap.wrap(par, 70):
                        print >> to_file,  u'\t' + l2
                else:
                    if parI < (len(paragraphs)-1):
                        print >> to_file
                if SPLIT_ON_BLANK_LINES:
                    print >> to_file
            if not SPLIT_ON_BLANK_LINES:
                print >> to_file
            self._flush()


bzrlib.log.register_formatter('gnu', GnuFormatter)

