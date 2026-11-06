using System;
using System.Net.Sockets;
using System.Text;
using System.Text.Json;

namespace TriviaClient
{
    public class Communicator
    {
        private TcpClient _client;
        private NetworkStream _stream;

        public bool IsConnected => _client?.Connected ?? false;

        public void Connect(string host, int port)
        {
            _client = new TcpClient();
            _client.Connect(host, port);
            _stream = _client.GetStream();
        }

        public void Disconnect()
        {
            _stream?.Close();
            _client?.Close();
        }

        // Send: [1 byte code] + [4 bytes length LE] + [JSON body]
        public void Send(byte code, object data = null)
        {
            string json = data == null ? "{}" : JsonSerializer.Serialize(data);
            byte[] body = Encoding.UTF8.GetBytes(json);
            byte[] length = BitConverter.GetBytes(body.Length); // little-endian because that's the default on Windows

            byte[] packet = new byte[1 + 4 + body.Length];
            packet[0] = code;
            Array.Copy(length, 0, packet, 1, 4);
            Array.Copy(body, 0, packet, 5, body.Length);

            _stream.Write(packet, 0, packet.Length);
        }

        // Receive: [1 byte code] + [4 bytes length LE] + [JSON body]
        public (byte code, JsonDocument doc) Receive()
        {
            byte[] codeBuf = ReadExactly(1);
            byte code = codeBuf[0];

            byte[] sizeBuf = ReadExactly(4);
            int size = BitConverter.ToInt32(sizeBuf, 0);

            byte[] body = ReadExactly(size);
            string json = Encoding.UTF8.GetString(body);

            var doc = JsonDocument.Parse(json);
            return (code, doc);
        }

        // Convenience: send then immediately receive
        public (byte code, JsonDocument doc) SendAndReceive(byte code, object data = null)
        {
            Send(code, data);
            return Receive();
        }

        private byte[] ReadExactly(int count)
        {
            byte[] buf = new byte[count];
            int total = 0;
            while (total < count)
            {
                int r = _stream.Read(buf, total, count - total);
                if (r == 0) throw new Exception("Server disconnected");
                total += r;
            }
            return buf;
        }
    }
}
