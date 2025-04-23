using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Igonin_Form
{
	internal class IgoninSessions : ViewModelBase
	{
		public enum MessageCommands
		{
			MT_START = 0,
			MT_STOP = 1,
			MT_DATA = 2,
			MT_EXIT = 3
		};

		[DllImport("Igonin_MMF_DLL.dll", CharSet = CharSet.Unicode)]
		public static extern void mapSend(int inx, int command, string msg);

		Process childProcess = null;
		//System.Threading.EventWaitHandle stopEvent = new EventWaitHandle(false, EventResetMode.AutoReset, "StopEvent");
		//System.Threading.EventWaitHandle startEvent = new EventWaitHandle(false, EventResetMode.AutoReset, "StartEvent");
		System.Threading.EventWaitHandle confirmEvent = new EventWaitHandle(false, EventResetMode.AutoReset, "ConfirmEvent");
		//System.Threading.EventWaitHandle closeEvent = new EventWaitHandle(false, EventResetMode.AutoReset, "CloseEvent");
		System.Threading.EventWaitHandle messageEvent = new EventWaitHandle(false, EventResetMode.AutoReset, "MessageEvent");

		public ObservableCollection<string> Sessions { get; set; } = new ObservableCollection<string>();
		
		int sessionIndex = 1;
		public int SessionsCount { get => sessionIndex; set { if (value < 1) SessionsCount = 1; 
															  else Set(ref sessionIndex, value); } }
		int sNumber = 0;

		string threatMessage = "clear";
		public string ThreatMessage { get => threatMessage; set => Set(ref threatMessage, value); }

		int selectedThreat = -1;
		public int SelectedThreat { get => selectedThreat; set => Set(ref selectedThreat, value); }

		public void StartSession()
		{
			if (childProcess == null || childProcess.HasExited) {
				childProcess = Process.Start("Igonin_LB1.exe");
				Sessions.Clear();
				Sessions.Add("Главный поток");
				Sessions.Add("Все потоки");
				//add "sessionsCount" sessions
			} else {
				for (int i = 0; i < SessionsCount; i++) {
					SendMessage(0, 0);
					confirmEvent.WaitOne();
					Sessions.Add($"Поток № {++sNumber}");
				}
			}

		}

		public void StopSession()
		{
			if (!(childProcess == null || childProcess.HasExited || (sNumber == 0))) {
				SendMessage(0, 1);
				confirmEvent.WaitOne();
				sNumber -= 1;
				Sessions.RemoveAt(sNumber + 2);
				if (sNumber == 0) {
					Sessions.Clear();
				}
			}
		}

		public void CloseSessions()
		{
			SendMessage(0, 3);
		}

		private void SendMessage(int inx, int command, string message = "")
		{
			//DEBUG
			mapSend(inx, command, message);
			//
			messageEvent.Set();
		}

		public void SendData()
		{
			if (!string.IsNullOrEmpty(ThreatMessage))
				if (SelectedThreat == 1)
					SendMessage(SelectedThreat, 2, ThreatMessage);
				else if (SelectedThreat > 1)
					SendMessage(SelectedThreat, 2, ThreatMessage);
		}

		//DEBUG
		public void CheckServer()
		{
			//var m = Message.send(2, MessageTypes.MT_INIT);
			var m = Message.send(10, MessageTypes.MT_INIT);
			MessageTypes msg = Message.receive(m); 
			ThreatMessage = msg.ToString();
		}
	}
}
