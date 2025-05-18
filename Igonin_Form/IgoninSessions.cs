using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Interop;
using System.Windows.Threading;
using System.Security.Cryptography.Xml;
using System.Windows;

namespace Igonin_Form
{
	internal class IgoninSessions : ViewModelBase
	{
		public enum MessageTypes : int
		{
			MT_INIT,
			MT_CLOSE,
			MT_GETDATA,
			MT_DATA,
			MT_NODATA,
			MT_CONFIRM,
			MT_EXIT
		};

		[StructLayout(LayoutKind.Sequential)]
		public struct MessageHeader
		{
			public int from;
			public int to;
			public int type;
			public int size;
		}

		[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
		public struct MessageData
		{
			public MessageHeader header;
			[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 250)]
			public string data;
		}


		[DllImport("Igonin_MMF_DLL.dll", CharSet = CharSet.Unicode)]
		public static extern MessageData getServerData(int from, MessageTypes command = MessageTypes.MT_GETDATA);

		[DllImport("Igonin_MMF_DLL.dll", CharSet = CharSet.Unicode)]
		public static extern void sendCommand(int from, int to, MessageTypes command, string msg="");

		public ObservableCollection<string> Clients { get; set; } = new ObservableCollection<string>();
		
		int sessionIndex = 1;
		public int SessionsCount { get => sessionIndex; set { if (value < 1) SessionsCount = 1; 
															  else Set(ref sessionIndex, value); } }
		int sNumber = 0;

		string sendingText = "";
		public string SendingText { get => sendingText; set => Set(ref sendingText, value); }

		string messageText = "";
		public string MessageText { get => messageText; set => Set(ref messageText, value); }

		int selectedClient = -1;
		public int SelectedClient { get => selectedClient; set => Set(ref selectedClient, value); }

		DispatcherTimer timer;

		int clientID = 0;
		public int ClientID { get => clientID; set => Set(ref clientID, value); }

		List<int> ClientsID = [0];

		public IgoninSessions()
		{
			timer = new DispatcherTimer();
			timer.Interval = TimeSpan.FromMilliseconds(100);
			timer.Tick += CheckServer;
			timer.Start();
		}

		private void InitClient()
		{
			//send Init msg, recieve id
			MessageData msgData = getServerData(0, MessageTypes.MT_INIT);
			
			ClientID = msgData.header.to;
		
		}

		public void CloseSessions()
		{
			sendCommand(ClientID, 0, MessageTypes.MT_CLOSE);
		}

		public void SendData()
		{
			if (!string.IsNullOrEmpty(SendingText) && (SelectedClient >= 0) && (SelectedClient != ClientID))
				if (SelectedClient > 0) {
					sendCommand(ClientID, ClientsID[SelectedClient - 1], MessageTypes.MT_DATA, SendingText);
				}
				else {
					sendCommand(ClientID, 0, MessageTypes.MT_DATA, SendingText);
				}

		}

		public void CheckServer(object? sender, EventArgs e)
		{
			bool tmp = true;
			if (ClientID == 0) {
				InitClient();
			}
			while (tmp) {
				MessageData msgData = getServerData(clientID);

				switch (msgData.header.type) {

					case (int)MessageTypes.MT_GETDATA:
						List<int> IDs = msgData.data.Split(' ', StringSplitOptions.RemoveEmptyEntries)
													.Select(int.Parse)
													.ToList();
						if (ClientsID.Count() != IDs.Count() || ClientsID.Last() < IDs.Last()) {
							UpdateSessions(IDs);
						}
						tmp = false;
						break;

					case (int)MessageTypes.MT_DATA:
						string message = "";
						message += $"Клиент №{msgData.header.from}: ";
						message += msgData.data + '\n';
						MessageText += message;
						break;

					default:
						break;
				}
			}
			//cnt = msgData.he
			//if (cnt != sNumber)
			//	UpdateSessions(cnt);
		}

		void UpdateSessions(List<int> IDs)
		{
			int tmp_sel = SelectedClient;
			Clients.Clear();
			Clients.Add("Все клиенты");
			foreach (int id in IDs) {
				Clients.Add($"Клиент № {id}");
			}
			ClientsID = new List<int>(IDs);
		}
	}
}
