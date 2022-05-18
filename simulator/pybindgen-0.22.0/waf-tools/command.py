from waflib import TaskGen# import feature, taskgen_method, before_method, task_gen
from waflib import Node, Task, Utils, Build
import subprocess
from waflib import Options

import shellcmd
#shellcmd.subprocess = pproc # the WAF version of the subprocess module is supposedly less buggy

from waflib.Logs import debug, error
shellcmd.debug = debug

from waflib import Task

import re


arg_rx = re.compile(r"(?P<dollar>\$\$)|(?P<subst>\$\{(?P<var>\w+)(?P<code>.*?)\})", re.M)

def nice_path(input):
	return input.path_from(input.ctx.launch_node())

class command(Task.Task):
	color = "BLUE"
	def __init__(self, env, generator):
		Task.Task.__init__(self, env=env, normal=1, generator=generator)

	def __str__(self):
		"string to display to the user"
		src_str = ' '.join([nice_path(a) for a in self.inputs])
		tgt_str = ' '.join([nice_path(a) for a in self.outputs])
		if self.outputs:
			sep = ' -> '
		else:
			sep = ''

		pipeline = shellcmd.Pipeline()
		pipeline.parse(self.generator.command)
		cmd = pipeline.get_abbreviated_command()
		return 'command (%s): %s%s%s\n' % (cmd, src_str, sep, tgt_str)

	def _subst_arg(self, arg, direction, namespace):
		"""
		@param arg: the command argument (or stdin/stdout/stderr) to substitute
		@param direction: direction of the argument: 'in', 'out', or None
		"""
		def repl(match):
			if match.group('dollar'):
				return "$"
			elif match.group('subst'):
				var = match.group('var')
				code = match.group('code')
				result = eval(var+code, namespace)
				if isinstance(result, Node.Node):
					if var == 'TGT':
						return result.get_bld().abspath()
					elif var == 'SRC':
						return result.srcpath()
					else:
						raise ValueError("Bad subst variable %r" % var)
				elif result is self.inputs:
					if len(self.inputs) == 1:
						return result[0].srcpath()
					else:
						raise ValueError("${SRC} requested but have multiple sources; which one?")
				elif result is self.outputs:
					if len(self.outputs) == 1:
						return result[0].get_bld().abspath()
					else:
						raise ValueError("${TGT} requested but have multiple targets; which one?")
				elif isinstance(result, list):
					assert len(result) == 1
					return result[0]
				else:
					return result
			return None

		return arg_rx.sub(repl, arg)

	def run(self):
		pipeline = shellcmd.Pipeline()
		pipeline.parse(self.generator.command)
		namespace = self.env.get_merged_dict()
		if self.generator.variables is not None:
			namespace.update(self.generator.variables)
		namespace.update(env=self.env, SRC=self.inputs, TGT=self.outputs)
		for cmd in pipeline.pipeline:
			if isinstance(cmd, shellcmd.Command):
				if isinstance(cmd.stdin, str):
					cmd.stdin = self._subst_arg(cmd.stdin, 'in', namespace)
				if isinstance(cmd.stdout, str):
					cmd.stdout = self._subst_arg(cmd.stdout, 'out', namespace)
				if isinstance(cmd.stderr, str):
					cmd.stderr = self._subst_arg(cmd.stderr, 'out', namespace)
				for argI in range(len(cmd.argv)):
					cmd.argv[argI] = self._subst_arg(cmd.argv[argI], None, namespace)
				if cmd.env_vars is not None:
					env_vars = dict()
					for name, value in list(cmd.env_vars.items()):
						env_vars[name] = self._subst_arg(value, None, namespace)
					cmd.env_vars = env_vars
			elif isinstance(cmd, shellcmd.Chdir):
				cmd.dir = self._subst_arg(cmd.dir, None, namespace)
		return pipeline.run(verbose=(Options.options.verbose > 0))

@TaskGen.taskgen_method
@TaskGen.feature('command')
def init_command(self):
	Utils.def_attrs(self,
					# other variables that can be used in the command: ${VARIABLE}
					variables = None,
					rule='')


@TaskGen.taskgen_method
@TaskGen.feature('command')
@TaskGen.before_method('process_source')
def process_rule(self):
	if not 'command' in self.features:
		return
	# now create one instance
	tsk = self.create_task('command')
	if getattr(self, 'target', None):
		if isinstance(self.target, str):
			self.target = self.target.split()
		if not isinstance(self.target, list):
			self.target = [self.target]
		for x in self.target:
			if isinstance(x, str):
				tsk.outputs.append(self.path.find_or_declare(x))
			else:
				x.parent.mkdir() # if a node was given, create the required folders
				tsk.outputs.append(x)
		if getattr(self, 'install_path', None):
			# from waf 1.5
			# although convenient, it does not 1. allow to name the target file and 2. symlinks
			# TODO remove in waf 1.7
			self.bld.install_files(self.install_path, tsk.outputs)

	if getattr(self, 'source', None):
		tsk.inputs = self.to_nodes(self.source)
		# bypass the execution of process_source by setting the source to an empty list
		self.source = []

	elif getattr(self, 'deps', None):
		def scan(self):
			nodes = []
			for x in self.generator.to_list(self.generator.deps):
				node = self.generator.path.find_resource(x)
				if not node:
					self.generator.bld.fatal('Could not find %r (was it declared?)' % x)
				nodes.append(node)
			return [nodes, []]
		cls.scan = scan

	setattr(tsk, "dep_vars", getattr(self, "dep_vars", None))
