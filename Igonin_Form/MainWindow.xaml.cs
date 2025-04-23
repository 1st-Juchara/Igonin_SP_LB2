using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Text;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Igonin_Form;


namespace Lab_1_Igonin
{
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window
	{

		
		IgoninSessions Sessions = new IgoninSessions();
		

		public MainWindow()
		{
			DataContext = Sessions;
			InitializeComponent();
		}


		private void Button_Start_Click(object sender, RoutedEventArgs e)
		{
			Sessions.StartSession();
		}

		private void Button_Stop_Click(object sender, RoutedEventArgs e)
		{
			Sessions.StopSession();
		}

		private void Button_Count_Up(object sender, RoutedEventArgs e)
		{
			Sessions.SessionsCount += 1;
		}

		private void Button_Count_Down(object sender, RoutedEventArgs e)
		{
			Sessions.SessionsCount -= 1;
		}
		private void Button_Send_Message(object sender, RoutedEventArgs e)
		{
			Sessions.SendData();
		}
		private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
		{
			Sessions.CloseSessions();
		}

		private void Button_Check_Server(object sender, RoutedEventArgs e)
		{
			Sessions.CheckServer();
		}
	}
}