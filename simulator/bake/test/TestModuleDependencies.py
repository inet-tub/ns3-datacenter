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
import unittest
# hack to save ourselves from having to use PYTHONPATH
import sys
import os
sys.path.append(os.path.join (os.getcwd(), '..'))
import bake

import re

class SrcTest:
    _name=''
    def __init__(self, name):
        self._name = name

class DepInfo:
    def __init__(self):
        self._edges = []
        self._optional_edges = []
    def add(self,src,dst, optional = False):
        self._optional_edges.append([src,dst, optional])
    def edges(self):
        return self._edges

class Test:
    def __init__(self, deps_str, failure_targets):
        # the syntax for deps_str encodes many things:
        # B depends on A: "A -> B"
        # When processing B, add new dependencies to dependency graph:
        # B {
        #  C -> D
        #  D -> B
        # }
        # B depends optionally on A: "A ?> B"
        # 
        self._deps = bake.Dependencies()
        self._infos = self._parse(deps_str, self._deps)
        self._failure = failure_targets
        self._processed = []
        
    def _parse(self,deps_str, deps):
        infos = dict()
        reg_edge = re.compile('([^ ]+) -> (.+)')
        reg_opt_edge = re.compile('([^ ]+) \?> (.+)')
        reg_open = re.compile('([^ ]+) {?')
        reg_close = re.compile('}?')
        current_info = None
        current_info_dst = ''
        for line in deps_str.splitlines():
            match_edge = reg_edge.search(line)
            match_opt_edge = reg_opt_edge.search(line)
            match_open = reg_open.search(line)
            match_close = reg_close.search(line)
            if match_edge is not None:
                src = match_edge.group(1)
                dst = match_edge.group(2)
                if current_info is None:
                    deps.add_dst(src)
                    deps.add_dst(dst)
                    deps.add_dep(src,dst)
                else:
                    current_info.add(src,dst)
                continue
            if match_opt_edge is not None:
                src = match_opt_edge.group(1)
                dst = match_opt_edge.group(2)
                if current_info is None:
                    deps.add_dst(src)
                    deps.add_dst(dst)
                    deps.add_dep(src,dst, optional = True)
                else:
                    current_info.add(src,dst, optional = True)
                continue
            if match_open is not None:
                dst = match_open.group(1)
                current_info = DepInfo()
                current_info_dst = dst;
                continue
            if match_close is not None:
                infos[current_info_dst] = current_info
                continue
        return infos
    
    def _dep_handler(self, target, context):
        if target in self._failure:
            return False
        self._processed.append(target)
        if self._infos.has_key(target):
            for src,dst,optional in self._infos[target].edges ():
                self._deps.add_dst(src)
                self._deps.add_dst(dst)
                self._deps.add_dep(src,dst, optional)
        return True
                
    def run(self, targets):
        self._deps.resolve(targets, self._dep_handler, None)
        return self._processed

class TestModuleDependencies(unittest.TestCase):
    """ Tests for the dependencies between modules. """
    
    def run_one_test(self, deps, targets, expected, failure = []):
        test = Test (deps, failure)
        got = test.run(targets)
        self.assertEqual(got, expected)
        
    def test_simple(self):
        deps = """
foo.h -> foo.c
foo.c -> foo.o
bar.h -> bar.c
bar.c -> bar.o
foo.o -> main
bar.o -> main
lex.yy -> lex.yy.h
lex.yy.h -> bar.c
"""
        self.run_one_test(deps, ['bar.o'], 
                          ['bar.h', 'lex.yy', 'lex.yy.h', 'bar.c', 'bar.o'])
        self.run_one_test(deps, ['foo.o'], 
                          ['foo.h', 'foo.c', 'foo.o'])
        self.run_one_test(deps, ['main'], 
                          ['bar.h', 'foo.h', 'lex.yy', 'foo.c', 'lex.yy.h', 
                           'bar.c', 'foo.o', 'bar.o', 'main'])
        self.run_one_test(deps, ['main', 'foo.o'], 
                          ['bar.h', 'foo.h', 'lex.yy', 'foo.c', 'lex.yy.h', 
                           'bar.c', 'foo.o', 'bar.o', 'main'])
        
    def Dtest_optional(self):
        self.run_one_test("A ?> B", targets = [SrcTest('B')],
                          expected = [SrcTest('A'), SrcTest('B')])
        self.run_one_test("A ?> B", targets = [SrcTest('B')],
                          expected = [SrcTest('B')],
                          failure = [SrcTest('A')])
        self.assertRaises(bake.DependencyUnmet, self.run_one_test, 
                          "A -> B", targets = [SrcTest('B')],
                          expected = [SrcTest('B')],
                          failure = [SrcTest('A')])
        self.run_one_test("""A ?> B
B -> C
""", targets = ['C'],
                          expected = ['B', 'C'],
                          failure = ['A'])
        self.run_one_test("""A ?> B
C ?> B
""", targets = ['B'],
                          expected = ['B'],
                          failure = ['A', 'C'])


        
if __name__ == '__main__':
    unittest.main()
