import unittest
import sunbeam

class TestParse(unittest.TestCase):

    def setUp(self):
        self.spe3 = 'spe3/SPE3CASE1.DATA'

    def parse(self):
        sunbeam.parse(self.spe3)
