using System;
using System.IO;
using System.Net.Sockets;
using System.Net;

namespace socket
{


	public class Server
	{
		public static void Main(string[] args)
		{
			Console.WriteLine("#####Server Here#####");

			DateTime date = DateTime.Now;
			try
			{
				ServerSocket echoServer = new ServerSocket(8888);

				using (Socket clientsocket = echoServer.accept())
				{

					Console.WriteLine("Server waiting for the Client");


					System.IO.StreamReader @is;
					@is = new System.IO.StreamReader(clientsocket.InputStream);

					PrintWriter os;
					os = new PrintWriter(new System.IO.StreamWriter(clientsocket.OutputStream));

					os.println("Let's communicate (\"Quit\" to disconnect)");
					os.flush();

					for (;;)
					{
						string inputLine = @is.ReadLine();
						string reverse = "";
						int wordcount = 1, lettercount = 0;
						int length = inputLine.Length;

						for (int i = length - 1 ; i >= 0 ; i--)
						{
							reverse = reverse + inputLine[i];

							if (inputLine[i] == ' ' && inputLine[i + 1] != ' ')
							{
								wordcount++;
							}
							if (inputLine[i] != ' ')
							{
								lettercount++;
							}

						}
						if (inputLine == null)
						{
							break;
						}
						else
						{
							os.println("Echoed from Server****\t" + "Original Message: " + inputLine + " \tReverse Message: " + reverse + "\tNo. of letters:" + lettercount + "\tNumber of words: " + wordcount + "\tCurrent Date & Time:" + date.ToString());
							os.flush();
							Console.WriteLine("Client : " + reverse);

							if (inputLine.Trim().Equals("QUIT") || inputLine.Trim().Equals("Quit"))
							{
								break;
							}
						}
					}
					@is.Close();
					os.close();
					clientsocket.close();
				}
				echoServer.close();

			}
			catch (Exception e)
			{
				Console.WriteLine("Error : " + e);
			}


		}
	}
}