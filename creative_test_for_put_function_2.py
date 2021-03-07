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


    def test_put_value_range(self):
        print("put value range test..")

        arr = bytearray()
        arr.extend(('put key=/=()(=/%!++/*\0').encode('UTF-8'))
        self.s.send(arr)
        data = self.s.recv(1024)
        data_str = str(data)
        # print(data_str)
        status, result = result_parsing(data_str)
        self.assertEqual("O", status, "Response status OK")
        self.assertEqual("OK", result, "The function working as expected")

        self.s.send(b"get key\0")
        data = self.s.recv(1024)
        data_str = str(data)
        # print(data_str)
        status, result = result_parsing(data_str)
        self.assertEqual("O", status, "Response status OK")
        self.assertEqual("/=()(=/%!++/*", result, "The function working as expected")

        self.s.send(b"del key\0")
        data = self.s.recv(1024)
        data_str = str(data)
        status, result = result_parsing(data_str)
        self.assertEqual("O", status, "Response status OK")
        self.assertEqual("OK", result, "The function working as expected")

    def test_put_value_range_version_2(self):
        print("put value range version 2 test..")

        arr = bytearray()
        arr.extend(('put key"=/=()(=/%!++/*"\0').encode('UTF-8'))
        self.s.send(arr)
        data = self.s.recv(1024)
        data_str = str(data)
        # print(data_str)
        status, result = result_parsing(data_str)
        self.assertEqual("O", status, "Response status OK")
        self.assertEqual("OK", result, "The function working as expected")

        self.s.send(b'get key"\0')
        data = self.s.recv(1024)
        data_str = str(data)
        # print(data_str)
        status, result = result_parsing(data_str)
        self.assertEqual("O", status, "Response status OK")
        self.assertEqual('/=()(=/%!++/*"', result, "The function working as expected")

        self.s.send(b'del key"\0')
        data = self.s.recv(1024)
        data_str = str(data)
        status, result = result_parsing(data_str)
        self.assertEqual("O", status, "Response status OK")
        self.assertEqual("OK", result, "The function working as expected")

    def test_put_key_range(self):
        print("put key range test..")
        arr = bytearray()
        arr.extend(('put /=()(=/%!++/*=1\0').encode('UTF-8'))
        self.s.send(arr)
        data = self.s.recv(1024)
        data_str = str(data)
        print(data_str)
        status, result = result_parsing(data_str)
        self.assertEqual("O", status, "Response status OK")
        self.assertEqual("OK", result, "The function working as expected")

        self.s.send(b'get /=()(=/%!++/*\0')
        data = self.s.recv(1024)
        data_str = str(data)
        print(data_str)
        status, result = result_parsing(data_str)
        self.assertEqual("E", status, "Response status Error")
        self.assertEqual("key not found", result, "The function working as expected")

        self.s.send(b'del /\0')
        data = self.s.recv(1024)
        data_str = str(data)
        print(data_str)
        status, result = result_parsing(data_str)
        self.assertEqual("O", status, "Response status OK")
        self.assertEqual("OK", result, "The function working as expected")

    def tearDown(self) -> None:
        print("-----------TearDown----------------------")
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
