import string
import threading
from dataclasses import dataclass
import socket, struct, time

MT_INIT		= 0
MT_CLOSE	= 1
MT_GETDATA	= 2
MT_DATA		= 3
MT_NODATA	= 4
MT_CONFIRM	= 5


@dataclass
class MsgHeader:

	From: int = 0
	To: int = 0
	Type: int = 0
	Size: int = 0

	def Send(self, s):
		s.send(struct.pack(f'iiii', self.From, self.To, self.Type, self.Size))

	def Receive(self, s):
		try:
			(self.From, self.To, self.Type, self.Size) = struct.unpack('iiii', s.recv(16))
		except:
			self.Size = 0
			self.Type = MT_NODATA

class Message:

	def __init__(self, To = 0, From = 0, Type = MT_DATA, Data=""):
		if (len(Data) > 0):
			self.Header = MsgHeader(To, From, Type, (len(Data) + 1) * 2)
		else:
			self.Header = MsgHeader(To, From, Type, 0)
		self.Data = Data

	def Send(self, s):
		self.Header.Send(s)
		if self.Header.Size > 0:
			s.send(struct.pack(f'{self.Header.Size}s', self.Data.encode('utf-16')))

	def Receive(self, s):
		self.Header.Receive(s)
		if self.Header.Size > 0:
			self.Data = struct.unpack(f'{self.Header.Size}s', s.recv(self.Header.Size))[0].decode('utf-16')
			
	def sendToServer(From, Type = MT_GETDATA, Data=""):
		HOST = 'localhost'
		PORT = 12345
		with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
			s.connect((HOST, PORT))
			m = Message(From, 0, Type, Data)
			m.Send(s)
			m.Receive(s)
			if m.Header.Type == MT_INIT:
				Message.ClientID = m.Header.To
			return m

	def MySend(From, To, Type, Data):
		HOST = 'localhost'
		PORT = 12345
		with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
			s.connect((HOST, PORT))
			m = Message(From, To, Type, Data)
			m.Send(s)


