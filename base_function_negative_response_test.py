import re
import socket
import unittest

def result_parsing(str):
    content = re.search("b'(.*)\Wx", str)
    result = content.group(1).split(" ", 1)

    return result[0], result[1]

class Test(unittest.TestCase):


    def setUp(self) -> None:
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # host = "192.168.56.111"
        host = socket.gethostname()
        port = 3333  # The same port as used by the server
        self.s.connect((host, port))


    def test_list_with_negative_response(self):
        print("List function testing..")
        self.s.send(b'list \0')
        data = self.s.recv(1024)
        data_str = str(data)
        # print(data_str)
        status, result = result_parsing(data_str)
        self.assertEqual("E", status, "Response status OK")
        self.assertEqual("command not found", result, "The function working as expected")

    def test_count_with_negative_response(self):
        print("Count function testing..")
        self.s.send(b'count \0')
        data = self.s.recv(1024)
        data_str = str(data)
        # print(data_str)
        status, result = result_parsing(data_str)
        self.assertEqual("E", status, "Response status OK")
        self.assertEqual("command not found", result, "The function working as expected")

    def test_get_with_negative_response1(self):
        print("Get function testing..")
        self.s.send(b'getkey\0')
        data = self.s.recv(1024)
        data_str = str(data)
        # print(data_str)
        status, result = result_parsing(data_str)
        self.assertEqual("E", status, "Response status OK")
        self.assertEqual("command not found", result, "The function working as expected")

    def test_get_with_negative_response2(self):
        print("Get function testing..")
        self.s.send(b'get key\0')
        data = self.s.recv(1024)
        data_str = str(data)
        # print(data_str)
        status, result = result_parsing(data_str)
        self.assertEqual("E", status, "Response status OK")
        self.assertEqual("key not found", result, "The function working as expected")

    def test_put_with_negative_response(self):
        self.s.send(b'putkey=1\0')
        data = self.s.recv(1024)
        data_str = str(data)
        # print(data_str)
        status, result = result_parsing(data_str)
        self.assertEqual("E", status, "Response status OK")
        self.assertEqual("command not found", result, "The function working as expected")

    def tearDown(self) -> None:
        print("-----------------tearDown---------------")
        self.s.send(b'count\0')
        data = self.s.recv(1024)
        data_str = str(data)
        status, result = result_parsing(data_str)
        print(data_str)
        print("-----------------------------------------")
        self.assertEqual("O", status, "Response status OK")
        self.assertEqual("0", result, "The function working as expected")
        self.s.close()


if __name__ == '__main__':
    unittest.main()
