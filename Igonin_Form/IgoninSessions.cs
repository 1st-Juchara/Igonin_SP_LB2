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

namespace Igonin_Form
{
	internal class IgoninSessions : ViewModelBase
	{
		//public enum MessageCommands
		//{
		//	MT_START = 0,
		//	MT_STOP = 1,
		//	MT_DATA = 2,
		//	MT_EXIT = 3
		//};

		//[DllImport("Igonin_MMF_DLL.dll", CharSet = CharSet.Unicode)]
		//public static extern void mapSend(int inx, int command, string msg);

		[DllImport("Igonin_MMF_DLL.dll", CharSet = CharSet.Unicode)]
		public static extern int getSessionCount();

		[DllImport("Igonin_MMF_DLL.dll", CharSet = CharSet.Unicode)]
		public static extern void sendCommand(int inx, MessageTypes command, string msg="");

		//Process childProcess = null;
		//System.Threading.EventWaitHandle stopEvent = new EventWaitHandle(false, EventResetMode.AutoReset, "StopEvent");
		//System.Threading.EventWaitHandle startEvent = new EventWaitHandle(false, EventResetMode.AutoReset, "StartEvent");
		//System.Threading.EventWaitHandle confirmEvent = new EventWaitHandle(false, EventResetMode.AutoReset, "ConfirmEvent");
		//System.Threading.EventWaitHandle closeEvent = new EventWaitHandle(false, EventResetMode.AutoReset, "CloseEvent");
		//System.Threading.EventWaitHandle messageEvent = new EventWaitHandle(false, EventResetMode.AutoReset, "MessageEvent");

		public ObservableCollection<string> Sessions { get; set; } = new ObservableCollection<string>();
		
		int sessionIndex = 1;
		public int SessionsCount { get => sessionIndex; set { if (value < 1) SessionsCount = 1; 
															  else Set(ref sessionIndex, value); } }
		int sNumber = 0;

		string threatMessage = "clear";
		public string ThreatMessage { get => threatMessage; set => Set(ref threatMessage, value); }

		int selectedThreat = -1;
		public int SelectedThreat { get => selectedThreat; set => Set(ref selectedThreat, value); }

		DispatcherTimer timer;

		int ticks = 0;

		public IgoninSessions()
		{
			timer = new DispatcherTimer();
			timer.Interval = TimeSpan.FromMilliseconds(100);
			timer.Tick += CheckServer;
			timer.Start();
		}

		public void StartSession()
		{
			//if (childProcess == null || childProcess.HasExited) {
			//	childProcess = Process.Start("Igonin_LB1.exe");
			//	Sessions.Clear();
			//	Sessions.Add("Главный поток");
			//	Sessions.Add("Все потоки");
			//	//add "sessionsCount" sessions
			//} else {
			//	for (int i = 0; i < SessionsCount; i++) {
			//		SendMessage(0, 0);
			//		confirmEvent.WaitOne();
			//		Sessions.Add($"Поток № {++sNumber}");
			//	}
			//}
			for (int i = 0; i < SessionsCount; i++) {
				sendCommand(0, MessageTypes.MT_INIT, "");
			}
		}

		public void StopSession()
		{
			if (sNumber > 0) {
				sendCommand(SelectedThreat, MessageTypes.MT_CLOSE);
			}
		}

		public void CloseSessions()
		{
			sendCommand(-1, MessageTypes.MT_EXIT);
		}

		public void SendData()
		{
			if (!string.IsNullOrEmpty(ThreatMessage) && (SelectedThreat >= 0))
				sendCommand(SelectedThreat, MessageTypes.MT_DATA, ThreatMessage);
		}

		//DEBUG
		public void CheckServer(object? sender, EventArgs e)
		{
			//var m = Message.send(2, MessageTypes.MT_INIT);
			//var m = Message.send(10, MessageTypes.MT_INIT);
			//sendCommand(10, MessageTypes.MT_INIT, "ABUBA");
			ticks += 1;
			int cnt = getSessionCount();
			if (cnt != sNumber)
				UpdateSessions(cnt);
		}

		void UpdateSessions(int cnt)
		{
			Sessions.Clear();
			Sessions.Add("Главный поток");
			Sessions.Add("Все потоки");
			for (int i = 0; i < cnt; i++) {
				Sessions.Add($"Поток № {i + 1}");
			}
			sNumber = cnt;
		}
	}
}
