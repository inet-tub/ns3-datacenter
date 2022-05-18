
==============================================
settings: pybindgen global settings
==============================================


.. automodule:: pybindgen.settings
    :members:
    :undoc-members:
    :show-inheritance:

.. data:: pybindgen.settings.wrapper_registry

  A :class:`pybindgen.wrapper_registry.WrapperRegistry` subclass to use for creating
  wrapper registries.  A wrapper registry ensures that at most one
  python wrapper exists for each C/C++ object.


.. autoclass:: pybindgen.wrapper_registry.WrapperRegistry
    :members:
    :undoc-members:
    :show-inheritance:
.. autoclass:: pybindgen.settings.NullWrapperRegistry
    :show-inheritance:
.. autoclass:: pybindgen.settings.StdMapWrapperRegistry
    :show-inheritance:
