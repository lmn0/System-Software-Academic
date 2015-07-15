using System;
using System.IO;
using System.Net.Sockets;
using System.Net;

namespace socket
{


	public class Client
	{
		public static void Main(string[] args)
		{
			try
			{
				Socket mySocket = new Socket("localhost", 8888);

				System.IO.StreamReader @is = new System.IO.StreamReader(mySocket.InputStream);
				PrintWriter os = new PrintWriter(mySocket.OutputStream, true);
				System.IO.StreamReader @out = new System.IO.StreamReader(System.in);

				Console.WriteLine("#####Client Here#####");

				string userInput;

				do
				{
					userInput = @is.ReadLine();

					if (userInput != null)
					{
						Console.WriteLine(userInput);
					}

					userInput = @out.ReadLine();
					os.println(userInput);
				}while (!userInput.Trim().Equals("QUIT") || !userInput.Trim().Equals("Quit"));

				@is.Close();
				os.close();
				@out.Close();
				mySocket.close();
			}
			catch (Exception e)
			{
				Console.Error.WriteLine(e);
			}
		}
	}
}