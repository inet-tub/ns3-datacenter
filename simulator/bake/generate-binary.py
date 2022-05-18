#!/usr/bin/env python3
###############################################################################
# Copyright (c) 2013 INRIA
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Authors: Daniel Camara  <daniel.camara@inria.fr>
#          Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
###############################################################################

def collect_source(source):
    import os
    sources = []
    for root, dirs, files in os.walk(source):
        for f in files:
            if f.endswith('.py'):
                sources.append(os.path.join(root, f))
    return sources

def calculate_hash(sources):
    import hashlib
    m = hashlib.sha256()
    for source in sources:
        f = open(source, 'r')
        for line in f:
            m.update(line.encode())
        f.close()
    sha256hash = m.hexdigest()
    return sha256hash

def generate_zip(sources):
    import zipfile
    import tempfile
    import os
    (handle, pathname) = tempfile.mkstemp()
    f = zipfile.ZipFile(pathname, 'w')
    for source in sources:
        f.write(source)
    f.close()
    os.close(handle)
    return pathname

def generate_binary(source_dir, output):
    import base64
    sources = collect_source(source_dir)
    sources_sha256 = calculate_hash(sources)
    zipfile = generate_zip(sources)
    with open(zipfile, "rb") as f:
        zipdata = base64.b64encode(f.read()).decode()
    
    f = open(output, 'w')
    f.write("""#!/usr/bin/env python3
sources = [%s]
sources_sha256 = "%s"
zipdata = \"\"\"%s\"\"\"

def decompress(output):
    import zipfile
    import base64
    decoded = base64.b64decode(zipdata).encode()
    f = zipfile.ZipFile(decoded, 'r')
    f.extractall(pathname)
    f.close()
    

import os
import sys
pathname = os.path.join('.bake', sources_sha256)
if not os.path.exists(pathname):
    os.makedirs(pathname)
    decompress(pathname)
elif not os.path.isdir(pathname):
    import tempfile
    pathname = tempfile.mkdtemp()
    decompress(pathname)
sys.path.append(pathname)
import %s as source
source.main(sys.argv)

""" % (','.join(["'%s'" % source for source in sources]), sources_sha256, zipdata,
       source_dir))
    f.close()

if __name__ == '__main__':
    import optparse
    parser = optparse.OptionParser()
    parser.add_option('-s', '--source', dest='source', type='string', action='store', default='bake',
                      help='Source directory to embed')
    parser.add_option('-o', '--output', dest='output', type='string', action='store', default='bake.binary',
                      help='Executable to generate')
    (options, args) = parser.parse_args()
    generate_binary(options.source, options.output)
    


    


