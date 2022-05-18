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
''' 
 Dependencies.py

 The purpose of this class is to capture a set of dependencies
 between a set of objects. The idea is that you have a set of 'targets'
 which depend on a set of sources. Each target can be the source of another
 target. There might be cycles but it's a bug and we need to detect it.

 Once we have all dependencies, we need to 'resolve' them. This means
 that we need to iterate over all targets and invoke a user-provided
 callback on each target. The tricky thing here is that the user-provided
 callback is allowed to recursively add new arbitrary dependencies, even
 to targets which have already been 'resolved' so, we need to be careful
 to re-resolve the targets to which dependencies have been recursively 
 added.
''' 

import copy
import sys
from bake.Exceptions import TaskError 
from bake.ModuleSource import SystemDependency

class CycleDetected:
    def __init__(self):
        return

class DependencyUnmet(Exception):
    def __init__(self, failed, method=''):
        self._failed = failed
        self._method = method
    def method(self):
        return self._method

    def failed(self):
        return self._failed

class DependencyLink():
    """ Stores information about the optional chain link of modules."""
    moduleProblem=False
    optionalChain=True
    module=None
    def __init__(self, optionalChain, module):
        self.optionalChain = optionalChain
        self.module = copy.copy(module)
        

class Target:
    """ Target modules meta information."""
        
    def __init__(self, dst, context):
        self._dst = dst
        self._src = []
        self._optional = dict()
        self._context = context
        self._dirty = True
    def is_dirty(self):
        return self._dirty
    def dirty(self):
        self._dirty = True
    def clean(self):
        self._dirty = False
    def add_src(self, src, optional):
        assert src not in self._src
        self._src.append(src)
        self._optional[src] = optional
    def dst(self):
        return self._dst
    def src(self):
        return self._src
    def is_src_optional(self,src):
        assert src in self._optional
        return self._optional[src]
    def context(self):
        return self._context

class Dependencies:
    def __init__(self): 
        # a dictionnary that maps a string (key) to the only instance
        # of the class Target that has this string as its target.
        self._targets = dict()
        # a dictionnary that maps a string (key) to the list of 
        # instances of the class Target that have this string in their
        # source list
        self._sources = dict()
        # the list of all items 
        self._items = []
        # Are we currently executing the resolve method. ?
        self._resolving = False
        # is there any target that is dirty ?
        self._dirty = False
        
    def add_dst(self, dst, context = None):
        """ Add the dependence"""
        
        # if the module passed as parameter, dst, is in fact a list of modules
        if isinstance(dst,list):
            return [self.add_dst(d,context) for d in dst]
        # the dependency is already recorded. nothing to do.
        if dst in self._targets:
            return
        # update dependency information
        target = Target(dst, context)
        self._targets[dst] = target
        # mark dirty target and its depending targets
        self._update_dirty(target)

    def add_dep(self, src, dst, optional = False):
        """ Registers a dependency regarding one module to another."""
        
        # if the dependence is in fact for a list of dependencies
        if isinstance(src,list):
            return [self.add_dep(s,dst) for s in src]
        assert dst in self._targets
        # the dependency is already recorded. nothing to do.
        target = self._targets[dst]
        if src in target.src ():
            return

        # record new dependency
        target = self._targets[dst]
        target.add_src(src, optional)
        if not src in self._sources:
            self._sources[src] = [target]
        elif target not in self._sources[src]:
            self._sources[src].append(target)

        # mark dirty target and its depending targets
        self._update_dirty(target)

#    def rec_dump(self,target):
#        """ Debugging purpose function to visualize the targets."""
#        str = ""
#        for src in target._dependencies:
#            str = str + " -> " + self.rec_dump(src)
#            
#        return target._name
#
#    def dump2(self,f,dot=True):
#        """ Debugging purpose function to visualize the targets."""
#        
#        f.write('digraph {\n')
#        for target in self._targets.values():
#            element = self.rec_dump(target.dst())
#            f.write('"' + target.dst()._name + '" -> ' + element + ';\n')


    def dump(self,f,dot=True):
        """ Debugging purpose function to visualize the targets."""
        
        f.write('digraph {\n')
        for target in self._targets.values():
            for src in target.src ():
                f.write('"' + src._name + '" -> "' + target.dst()._name + '";\n')
        f.write('}')

    def resolve(self, targets, callback = None, n=1):
        """ Resolve dependencies wrapper function."""
        
        # raise exceptions to signal errors:
        # todo:
        #  CycleDetected ()
        #  DependencyUnmet ()
        
        if isinstance(targets,str):
            targets = [targets]
            
        self._resolving = True
        if n == 1:
            self._resolve_serial(targets, callback)
        else:
            self._resolve_parallel(targets, callback, n)
            
        self._resolving = False

    def _update_dirty(self,target):
        """Registers dependency added modules for later treatment."""
        
        if self._resolving:
            depending = self._depend_on([target])
            for i in depending:
                i.dirty()
            self._dirty = True

    # return list of targets which depend on the input
    # target, including the input target itself.
    def _depend_on(self,targets):
        """ Finds the list of modules that depends on the target module."""
        
        workqueue = copy.copy(targets)
        deps = []
        while len(workqueue) > 0:
            i = workqueue.pop()
            if i not in deps:
                deps.append(i)
            if i.dst() in self._sources:
                workqueue.extend(self._sources[i.dst()])
        return deps

    # return list of targets which need to be resolved
    # to resolve the input targets
    def _dependencies_of(self,targets):
        """ Finds the list of dependencies of the target module."""
        
        # XXX: should detect cycles here.
        workqueue = [self._targets[target] 
                     for target in targets 
                        if target in self._targets]
        
        deps = []
        while len(workqueue) > 0:
            i = workqueue.pop()
            if i not in deps:
                deps.append(i)

            for src in i.src():
                if src in self._targets:
                    workqueue.append(self._targets[src])
        return deps

    def _is_leaf(self, target):
        """ Verifies if the target is independent of any module."""
        
        assert target.dst() in self._targets
        # a 'leaf' is a target which either has
        # no source or whose sources are not
        # targets themselves.
        for src in target.src():
            if src in self._targets:
                return False
        return True

    # return sorted list of targets such that the first
    # items must be 'resolved' first.
    def _sort(self,targets):
        """ Organize the modules putting on the head the resolved ones."""
        
        # to calculate this, we first collect the set of targets to
        # 'resolve'. i.e., the targets that 'targets' depends upon.
        to_resolve = self._dependencies_of(targets)
        
        # then, we collect the set of targets that are the leaves
        # of the dependency graph to initialize our workqueue
        leaves = [i for i in to_resolve if self._is_leaf(i)]
        workqueue = leaves
        prio = dict()
        
        # let's initialize the piority of every item to zero.
        for work in to_resolve:
            prio[work] = 0
            
        # and, now, we update the priority so that the
        # deepest targets of the dependency tree have the highest
        # priority.
        while len(workqueue) > 0:
            source = workqueue.pop()
            if not source.dst() in self._sources:
                continue
            for dst in self._sources[source.dst()]:
                if dst not in to_resolve:
                    continue
                prio[dst] = max(prio[dst], prio[source] + 1)
                workqueue.append(dst)

        # now, build an inverted dictionary of priorities
        # we want to find the list of targets for each priority
        prio_inverted = dict()
        for target in to_resolve:
            if prio[target] in prio_inverted:
                prio_inverted[prio[target]].append(target)
            else:
                prio_inverted[prio[target]] = [target]
               
        # generate a sorted list of targets, lowest-priority first
        sorted_targets = []
        for key in sorted(prio_inverted.keys()):
            sorted_targets.extend(sorted(prio_inverted[key], key=self.cmp_to_key(self._cmp)))
        # convert the list of targets into a list of steps
        return sorted_targets
    def cmp_to_key(self, mycmp):
        'Convert a cmp= function into a key= function'
        class K(object):
            def __init__(self, obj, *args):
                self.obj = obj
            def __lt__(self, other):
                return mycmp(self.obj, other.obj) < 0
            def __gt__(self, other):
                return mycmp(self.obj, other.obj) > 0
            def __eq__(self, other):
                return mycmp(self.obj, other.obj) == 0
            def __le__(self, other):
                return mycmp(self.obj, other.obj) <= 0
            def __ge__(self, other):
                return mycmp(self.obj, other.obj) >= 0
            def __ne__(self, other):
                return mycmp(self.obj, other.obj) != 0
        return K


    def _cmp(self, a, b):
        return (id(b.dst())-id(a.dst()));

    def _is_clean(self,targets):
        """ Returns true if the target is clean, resolved, and False if it 
        is dirty, i.e. not all the dependencies resolved yet.
        """

        for target in targets:
            if target in self._targets:
                if self._targets[target].is_dirty():
                    return False
        return True

    def _resolve_one_iteration(self, targets, callback):
        """ 'resolve' all targets which the input targets depend upon
        in the right order. If resolving one of these targets
        creates new targets, the function is interrupted and returns
        False. Otherwise, the function completes and returns True.
        """
        
        self._dirty = False
        
        # sort in a way that the nodes that have no dependencies are first
        queue = self._sort(targets)
        dirty = [i for i in queue if i.is_dirty()]
        for i in dirty:
            i.clean();
            assert self._is_clean(i.src())
            success = True
            if callback is None and i.context() is not None:
                try:
                    success = i.context()()
                except TaskError as e:
                    success = False
                    print("  > Error: " + e._reason)
#                except SystemExit as e:
#                    print(sys.exc_info())
#
#                    success = False
#                    print ("  > Error: " + e._reason)
                except:
                    success = False
                    import sys
                    er = sys.exc_info()[1]
                    print("  > Error: " + str(er))
                    from bake.ModuleEnvironment import ModuleEnvironment
                    if ModuleEnvironment._stopOnError:
                        er = sys.exc_info()[1]
                        sys.exit(1)
                    
            elif callback is not None:
                try:
                    success = callback(i.dst(), i.context())
                except TaskError as e:
                    success = False
                    print("  > Error: " + e._reason)
                    from bake.ModuleEnvironment import ModuleEnvironment
                    if ModuleEnvironment._stopOnError:
                        er = sys.exc_info()[1]
                        sys.exit(1)
                except:
                    success = False
                    import sys
                    er = sys.exc_info()[1]
                    print("  > Unexpected error: " + str(er))
                    from bake.ModuleEnvironment import ModuleEnvironment
                    if ModuleEnvironment._stopOnError:
                        er = sys.exc_info()[1]
                        sys.exit(1)
            if not success:
                if not i.dst() in self._sources:
                    raise DependencyUnmet(i.dst())
                else:
                    for j in self._sources[i.dst()]:
                        dependencyTmp= self.dependencies.get(i.dst()._name)
                        if dependencyTmp:
                            if isinstance(i.dst()._source, SystemDependency):
                                tailError =  'not available'
                            else:
                                tailError =  'failed'
                                
                            if not dependencyTmp.optionalChain:
                                raise DependencyUnmet(i.dst(), tailError)
                            
                            if not self.dependencies[i.dst()._name].moduleProblem:
                                
                                print(' > Problem: Optional dependency,'
                                             ' module "%s" %s\n'
                                             '   This may reduce the  '
                                             'functionality of the final build. \n'
                                             '   However, bake will continue since'
                                             ' "%s" is not an essential dependency.\n'
                                             '   For more'
                                             ' information call bake with -v or -vvv, for full verbose mode.\n' 
                                             % (i.dst()._name,tailError, i.dst()._name))
                                self.dependencies[i.dst()._name].moduleProblem = True
            if self._dirty:
                self._dirty = False
                return False
        return True

    def _resolve_serial(self, targets, callback):
        """ Resolves the dependencies in serial mode."""
        
        finished = self._resolve_one_iteration(targets, callback)
        while not finished:
            finished = self._resolve_one_iteration(targets, callback)

    def _resolve_parallel(self, targets, callback, n):
        """ Resolves the dependencies in parallel mode. Not yet functional."""
        
        # todo: implement parallel version
        self._resolve_serial(targets, callback)
        
    
    dependencies=dict()
    modTmp = dict()
    def recDependencies(self, targetModule, optionalDepChain):
        existModule = self.dependencies.get(targetModule._name)
        
        # verify if the module exist or not if does not insert it as a dependency
        if not existModule:
            self.dependencies[targetModule._name] = DependencyLink(optionalDepChain, targetModule)
        elif existModule.optionalChain and not optionalDepChain : # if it exists check if the 
            self.dependencies[targetModule._name].optionalChain = optionalDepChain
            
        if targetModule._dependencies:
            for j in targetModule._dependencies:
                if j._optional:
                    optionalDepChain= j._optional
                self.recDependencies(self.modTmp[j._name], optionalDepChain)
        

        
    def checkDependencies(self, targets, modules):
        enabled = copy.copy(targets)
        
        for i in modules:
            self.modTmp[i._name]=i
        
        for i in enabled:
            self.dependencies[i._name]= DependencyLink(False, i)
            if i._dependencies:
                for j in i._dependencies:
                    self.recDependencies(self.modTmp[j._name], j._optional)
        
        return self.dependencies

