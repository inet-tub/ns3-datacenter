import sys
import weakref
import gc
import os.path
import copy
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                '..', '..', 'build', 'tests', 'boost'))
import bar

import unittest


class TestBar(unittest.TestCase):

    def test_basic_gc(self):
        count0 = bar.Foo.instance_count

        f = bar.Foo("hello")
        self.assertEqual(bar.Foo.instance_count, count0 + 1)
        self.assertEqual(f.get_datum(), "hello")
        del f
        while gc.collect():
            pass
        self.assertEqual(bar.Foo.instance_count, count0)


    def test_function_takes_foo(self):
        count0 = bar.Foo.instance_count

        f = bar.Foo("hello123")
        self.assertEqual(bar.Foo.instance_count, count0 + 1)
        self.assertEqual(f.get_datum(), "hello123")
        bar.function_that_takes_foo(f)
        del f
        while gc.collect():
            pass
        self.assertEqual(bar.Foo.instance_count, count0+1) # the object stays alive
    

        f1 = bar.function_that_returns_foo()
        self.assertEqual(f1.get_datum(), "hello123")

        self.assertEqual(bar.Foo.instance_count, count0+1)
        del f1
        while gc.collect():
            pass
        self.assertEqual(bar.Foo.instance_count, count0+1) # the object stays alive


    def test_class_takes_foo(self):
        count0 = bar.Foo.instance_count


        f = bar.Foo("hello12")
        self.assertEqual(bar.Foo.instance_count, count0 + 1)
        self.assertEqual(f.get_datum(), "hello12")
        takes = bar.ClassThatTakesFoo(f)
        del f
        while gc.collect():
            pass
        self.assertEqual(bar.Foo.instance_count, count0+1) # the object stays alive
    

        f1 = takes.get_foo()
        self.assertEqual(f1.get_datum(), "hello12")

        self.assertEqual(bar.Foo.instance_count, count0+1)
        del f1, takes
        while gc.collect():
            pass
        self.assertEqual(bar.Foo.instance_count, count0)


    def test_class_takes_foo_subclassing(self):

        count0 = bar.Foo.instance_count


        f = bar.Foo("hello45")
        self.assertEqual(bar.Foo.instance_count, count0 + 1)
        self.assertEqual(f.get_datum(), "hello45")

        class Takes(bar.ClassThatTakesFoo):
            def get_modified_foo(self, foo):
                d = foo.get_datum()
                return bar.Foo(d+"xxx")

        takes = Takes(f)
        del f
        while gc.collect():
            pass
        self.assertEqual(bar.Foo.instance_count, count0+1)
    

        f1 = takes.get_foo()
        self.assertEqual(f1.get_datum(), "hello45")

        self.assertEqual(bar.Foo.instance_count, count0+1)

        f2 = bar.Foo("helloyyy")
        self.assertEqual(bar.Foo.instance_count, count0+2)
        f3 = takes.get_modified_foo(f2)
        self.assertEqual(bar.Foo.instance_count, count0+3)
        self.assertEqual(f3.get_datum(), "helloyyyxxx")

        del f1, f2, f3, takes
        while gc.collect():
            pass
        self.assertEqual(bar.Foo.instance_count, count0)


if __name__ == '__main__':
    unittest.main()
