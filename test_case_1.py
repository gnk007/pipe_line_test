import socket
import unittest


class Test(unittest.TestCase):

    def setUp(self) -> None:
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # host = "192.168.56.111"
        host = socket.gethostname()
        port = 3333  # The same port as used by the server
        self.s.connect((host, port))
        self.s.send(b'put key=2\0')

    def test_get(self):
        self.s.send(b"count\0")
        data = self.s.recv(1024)
        print('Received', data)
        data_str = str(data)
        print('Received_str', data_str)
        self.assertEqual("1",data_str[4], "OKE")

    def tearDown(self) -> None:
        self.s.close()


if __name__ == '__main__':
    unittest.main()
