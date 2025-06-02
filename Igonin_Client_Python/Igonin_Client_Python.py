# -*- coding: utf-8 -*-

from email.policy import default
from http import client
from re import S, match
from subprocess import IDLE_PRIORITY_CLASS
import threading
from dataclasses import dataclass
import socket, struct, time
from tkinter import Menu
from Message import *
import re
import signal


#sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

class Session:

	START = 0
	SEND_MESSAGES = 1
	VIEW_MEASSAGES = 2
	VIEW_CLIENTS = 3
	
	client_state = 0
	clients_dict = {}

	client_ID = 0
	working = False

	def start(self):
		self.thread = threading.Thread(target=self.Check_server, daemon=True)
		self.working = True
		self.thread.start()

	def Process_menu(this):
	#menuCode = 0
	#menuCode = int(input("\n> "))

		if (this.client_state == this.START):
			print("----------------------------")
			print("1. Send message")
			print("2. View messages")
			print("3. View clients")
			print("4. Exit")

		elif (this.client_state == this.SEND_MESSAGES):
			print("----------------------------");
			print("Clients list:")
			i = 0
			for client in this.clients_dict:
				i += 1
				print(f"{i}. Client {client}")
			print("0. All Clients")

		elif (this.client_state == this.VIEW_MEASSAGES):
			print("----------------------------");
			print("Messages:")
			i = 0
			for client in this.clients_dict:
				if (this.clients_dict[client] != "0"):
					print(f"Client {client}: {this.clients_dict[client]}")

		elif (this.client_state == this.VIEW_CLIENTS):
			print("----------------------------");
			print("Clients list:")
			i = 0
			for client in this.clients_dict:
				i += 1
				print(f"{i}. Client {client}")
		else:
			print("ERROR: client state is strange!")

	def Check_server(this):
		
		while (True and this.working):
			t = True
			while (t):
				msgData = Message.sendToServer(this.client_ID)
	
				if (msgData.Header.Type == MT_DATA):
	
					id_from = msgData.Header.From
					if id_from not in this.clients_dict:
						this.clients_dict[id_from] = []
					this.clients_dict[msgData.Header.From] += " " + msgData.Data
	
				elif (msgData.Header.Type == MT_GETDATA):
	
					clean_string = re.sub(r'[^\d\s-]', '', msgData.Data)

					idToKill = {}
					for currentID in this.clients_dict:
						idToKill[currentID] = True

					for x in clean_string.split():
						try:
							ID = int(x)
							if ID not in this.clients_dict:
								this.clients_dict[ID] = ""
							else:
								idToKill[ID] = False

						except ValueError:
							print(f"Error parse: '{x}'")

					for ID in idToKill:
						if (idToKill[ID]):
							this.clients_dict.pop(ID, None)

					t = False
			time.sleep(0.5)

	def Process_exit(this):
		#Message.send(ClientID, 0, MessageTypes.MT_CLOSE);
		#Message.MySend(this.client_ID, 0, MT_CLOSE, "");
		this.working = False;

	def ProcessMessages(this):

		while True:
			this.Process_menu();
			if (this.client_state == this.START):
				menuCode = int(input("\n> "))
				if (menuCode == 4):
					this.Process_exit();
					return
				else:
					this.client_state = menuCode;
	
			elif (this.client_state == this.SEND_MESSAGES):
				menuCode = int(input("\n> "))
				if (menuCode >= 0 and menuCode <= len(this.clients_dict)):
					new_client_id = menuCode;
					print("Enter message:");
					msg = input("> ");
					Message.MySend(this.client_ID, new_client_id, MT_DATA, msg);
				this.client_state = this.START
	
			elif (this.client_state == this.VIEW_MEASSAGES):
				this.client_state = this.START;
	
			elif (this.client_state == this.VIEW_CLIENTS):
				this.client_state = this.START;
	
			#m = Message.SendMessage(MR_BROKER, MT_GETDATA)
			#if m.Header.Type == MT_DATA:
			#	print(m.Data)
			#else:
			#	time.sleep(1)

def closeClient(session : Session):
	def _close(signum, frame):
		session.Process_exit()
	return _close;

def notifyServer(session : Session):
	session.Check_server()
	time.sleep(1)


def Client():
	session = Session()
	msg = Message.sendToServer(0, MT_INIT)
	session.client_ID = msg.Header.To
	session.start()
	print(f"Session {session.client_ID} is init")

	t = threading.Thread(target=session.ProcessMessages())
	t.start()


if __name__ == '__main__':
	#locale.setlocale(locale.LC_ALL, 'ru_RU.UTF-8')
	Client()

