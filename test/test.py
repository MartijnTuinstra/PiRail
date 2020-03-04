from ctypes import *

import unittest
import os 


dir_path = os.path.dirname(os.path.realpath(__file__))

baanc = CDLL(dir_path + "/../baan.so")

class TestA (unittest.TestCase):

    def setUp(self):
        self.c = CDLL(dir_path + "/../baan.so")

        filename = c_char_p(b"python_test.txt")
        self.c.init_logger(filename)
        self.c.set_level(5)

        self.c.init_main()

    def tearDown(self):
        self.c.exit_logger()
        self.c.print_allocs()
        del self.c

    def basicTest(self):
        print("BasicTest")
        logfile = c_char_p(b"basicTest")
        logdetails = c_char_p(b"PythonTEST LOG")

        self.c.floggerf(3, logfile, 123, logdetails)

    def loadTest(self):
        self.c.read_module_Config(0)

if __name__ == "__main__":
    unittest.main()