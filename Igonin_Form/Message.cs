using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Net;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace Igonin_Form
{
	public enum MessageTypes : int
	{
		MT_INIT,
		MT_CLOSE,
		MT_GETDATA,
		MT_DATA,
		MT_NODATA,
		MT_CONFIRM
	};


	[StructLayout(LayoutKind.Sequential)]
	struct MessageHeader
	{
		[MarshalAs(UnmanagedType.I4)]
		public int from;
		[MarshalAs(UnmanagedType.I4)]
		public int to;
		[MarshalAs(UnmanagedType.I4)]
		public MessageTypes type;
		[MarshalAs(UnmanagedType.I4)]
		public int size;
	};

	class Message
	{
		public MessageHeader header;
		public string data;
		static int clientID;
		static Encoding? cp866 = null;

		Encoding get866()
		{
			if (cp866 is null) {
				Encoding.RegisterProvider(CodePagesEncodingProvider.Instance);
				cp866 = Encoding.GetEncoding("Unicode");
			}
			return cp866;
		}


		public Message(int from, int to, MessageTypes type = MessageTypes.MT_DATA, string data = "")
		{
			this.data = data;
			header = new MessageHeader() { from = from, to = to, type = type, size = data.Length * 2 };
		}

		static byte[] toBytes(object obj)
		{
			int size = Marshal.SizeOf(obj);
			byte[] buff = new byte[size];
			IntPtr ptr = Marshal.AllocHGlobal(size);
			Marshal.StructureToPtr(obj, ptr, true);
			Marshal.Copy(ptr, buff, 0, size);
			Marshal.FreeHGlobal(ptr);
			return buff;
		}

		static T fromBytes<T>(byte[] buff) where T : struct
		{
			T data = default(T);
			int size = Marshal.SizeOf(data);
			IntPtr i = Marshal.AllocHGlobal(size);
			Marshal.Copy(buff, 0, i, size);
			var d = Marshal.PtrToStructure(i, data.GetType());
			if (d is not null) {
				data = (T)d;
			}
			Marshal.FreeHGlobal(i);
			return data;
		}

		void send(Socket s)
		{
			s.Send(toBytes(header), Marshal.SizeOf(header), SocketFlags.None);
			if (header.size != 0) {
				s.Send(get866().GetBytes(data), header.size, SocketFlags.None);
			}
		}

		MessageTypes receive(Socket s)
		{
			byte[] buff = new byte[Marshal.SizeOf(header)];
			if (s.Receive(buff, Marshal.SizeOf(header), SocketFlags.None) == 0) {
				return MessageTypes.MT_NODATA;
			}
			header = fromBytes<MessageHeader>(buff);
			if (header.size > 0) {
				byte[] b = new byte[header.size];
				s.Receive(b, header.size, SocketFlags.None);
				data = get866().GetString(b, 0, header.size);

			}
			return header.type;
		}

		public static void send(Socket s, int from, int to, MessageTypes type = MessageTypes.MT_DATA, string data = "")
		{
			new Message(from, to, type, data).send(s);
		}

		public static void send(int from, int to, MessageTypes type = MessageTypes.MT_DATA, string data = "")
		{
			int nPort = 12345;
			IPEndPoint endPoint = new IPEndPoint(IPAddress.Parse("127.0.0.1"), nPort);
			Socket s = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
			s.Connect(endPoint);
			if (!s.Connected) {
				throw new Exception("Connection error");
			}
			var m = new Message(from, to, type, data);
			m.send(s);
		}
		public static Message sendToServer(int from, MessageTypes type = MessageTypes.MT_GETDATA, string data = "")
		{
			int nPort = 12345;
			IPEndPoint endPoint = new IPEndPoint(IPAddress.Parse("127.0.0.1"), nPort);
			Socket s = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
			s.Connect(endPoint);
			if (!s.Connected) {
				throw new Exception("Connection error");
			}
			var m = new Message(from, 0, type, data);
			m.send(s);
			m.receive(s);
			return m;
		}
	}

}
