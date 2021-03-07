import re
import socket
import unittest


def result_parsing(str):
    content = re.search("b'(.*)\Wx", str)
    result = content.group(1).split(" ", 1)

    return result[0], result[1]


class Test(unittest.TestCase):

    def setUp(self) -> None:
        print("Runing set up part")
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # host = "192.168.56.111"
        host = socket.gethostname()
        port = 3333
        self.s.connect((host, port))

        print("Set up part include put function testing")
        self.s.send(b'put key=2\0')
        data1 = self.s.recv(1024)
        data_str1 = str(data1)
        status, result = result_parsing(data_str1)
        self.assertEqual("O", status, "Response status OK")
        self.assertEqual("OK", result, "The function working as expected")

    def test_count(self):
        print("Count function testing..")
        self.s.send(b"count\0")
        data = self.s.recv(1024)
        data_str = str(data)
        status, result = result_parsing(data_str)
        self.assertEqual("O", status, "Response status OK")
        self.assertEqual("1", result, "The function working as expected")

    def test_get(self):
        print("Get function testing..")
        self.s.send(b"get key\0")
        data = self.s.recv(1024)
        data_str = str(data)
        status, result = result_parsing(data_str)
        self.assertEqual("O", status, "Response status OK")
        self.assertEqual("2", result, "The function working as expected")

    def test_list(self):
        print("List function testing..")
        self.s.send(b"list\0")
        data = self.s.recv(1024)
        data_str = str(data)
        status, result = result_parsing(data_str)
        self.assertEqual("O", status, "Response status OK")
        self.assertEqual("key", result, "The function working as expected")

    def tearDown(self) -> None:
        print("---------tearDown-----------")
        self.s.send(b"del key\0")
        data = self.s.recv(1024)
        data_str = str(data)
        status, result = result_parsing(data_str)
        self.assertEqual("O", status, "Response status OK")
        self.assertEqual("OK", result, "The function working as expected")
        self.s.send(b'count\0')
        data = self.s.recv(1024)
        data_str = str(data)
        status, result = result_parsing(data_str)
        print(data_str)
        print("-----------------------------")
        self.assertEqual("O", status, "Response status OK")
        self.assertEqual("0", result, "The function working as expected")
        self.s.close()


if __name__ == '__main__':
    unittest.main()
