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

		[DllImport("Igonin_MMF_DLL.dll", CharSet = CharSet.Unicode)]
		public static extern void mapSend(int inx, string msg);

		Process childProcess = null;
		System.Threading.EventWaitHandle stopEvent = new EventWaitHandle(false, EventResetMode.AutoReset, "StopEvent");
		System.Threading.EventWaitHandle startEvent = new EventWaitHandle(false, EventResetMode.AutoReset, "StartEvent");
		System.Threading.EventWaitHandle confirmEvent = new EventWaitHandle(false, EventResetMode.AutoReset, "ConfirmEvent");
		System.Threading.EventWaitHandle closeEvent = new EventWaitHandle(false, EventResetMode.AutoReset, "CloseEvent");
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
					startEvent.Set();
					confirmEvent.WaitOne();
					Sessions.Add($"Поток № {++sNumber}");
				}
			}

		}

		public void StopSession()
		{
			if (!(childProcess == null || childProcess.HasExited || (sNumber == 0))) {
				stopEvent.Set();
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
			closeEvent.Set();
		}

		private void SendMessage(int inx, string message)
		{
			if (!(childProcess == null || childProcess.HasExited || (sNumber == 0) || (string.IsNullOrEmpty(message)))) {
				mapSend(inx, message);
				messageEvent.Set();
				confirmEvent.WaitOne();
			}
		}

		public void SendMessageToThreat()
		{
			if (SelectedThreat == 0)
				SendMessage(SelectedThreat, ThreatMessage);
			else if (SelectedThreat > 1)
				SendMessage(SelectedThreat - 1, ThreatMessage);
		}
	}
}
