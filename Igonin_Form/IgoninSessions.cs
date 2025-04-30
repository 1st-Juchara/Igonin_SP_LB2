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

		[DllImport("Igonin_MMF_DLL.dll", CharSet = CharSet.Unicode)]
		public static extern int getSessionCount();

		[DllImport("Igonin_MMF_DLL.dll", CharSet = CharSet.Unicode)]
		public static extern void sendCommand(int inx, MessageTypes command, string msg="");

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
			for (int i = 0; i < SessionsCount; i++) {
				sendCommand(0, MessageTypes.MT_INIT, "");
			}
		}

		public void StopSession()
		{
			sendCommand(SelectedThreat, MessageTypes.MT_CLOSE);
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

		public void CheckServer(object? sender, EventArgs e)
		{
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
