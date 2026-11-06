using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text.Json;
using System.Windows.Forms;

namespace TriviaClient
{
    public class JoinRoomForm : Form
    {
        private readonly Communicator _comm;
        private readonly string _username;

        private ListBox lstRooms;
        private ListBox lstPlayers;
        private Button btnJoin, btnRefresh, btnLeave;
        private Label lblStatus, lblRoomName, lblPlayerCount;
        private GroupBox grpRooms, grpLobby;
        private System.Windows.Forms.Timer _refreshTimer;
        private Dictionary<int, string> _roomDictionary = new Dictionary<int, string>();
        private int _currentRoomId = -1;
        private string _currentRoomName = "";
        private bool _gameStarted = false;

        public JoinRoomForm(Communicator comm, string username)
        {
            _comm = comm;
            _username = username;
            InitializeComponents();
            LoadRooms();
            StartAutoRefresh();
        }

        private void InitializeComponents()
        {
            this.Text = "Trivia Client - Join Room";
            this.Size = new Size(450, 550);
            this.StartPosition = FormStartPosition.CenterParent;
            this.FormBorderStyle = FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.BackColor = Color.FromArgb(45, 45, 48);

            // Available Rooms Group
            grpRooms = new GroupBox()
            {
                Text = "Available Rooms",
                Location = new Point(12, 12),
                Size = new Size(410, 250),
                ForeColor = Color.White
            };

            Label lblInstruction = new Label()
            {
                Text = "Select a room and click 'Join':",
                Location = new Point(10, 20),
                Size = new Size(390, 20),
                ForeColor = Color.White
            };
            grpRooms.Controls.Add(lblInstruction);

            lstRooms = new ListBox()
            {
                Location = new Point(10, 45),
                Size = new Size(390, 130),
                BackColor = Color.FromArgb(30, 30, 30),
                ForeColor = Color.White,
                Font = new Font("Segoe UI", 10)
            };
            lstRooms.SelectedIndexChanged += (s, e) =>
            {
                btnJoin.Enabled = lstRooms.SelectedIndex >= 0 && lstRooms.Items.Count > 0 && !lstRooms.Items[0].ToString().Contains("No rooms");
            };
            grpRooms.Controls.Add(lstRooms);

            btnRefresh = new Button()
            {
                Text = "Refresh",
                Location = new Point(10, 185),
                Size = new Size(190, 35),
                BackColor = Color.FromArgb(70, 70, 70),
                ForeColor = Color.White,
                FlatStyle = FlatStyle.Flat,
                Cursor = Cursors.Hand
            };
            btnRefresh.Click += (s, e) => LoadRooms();
            grpRooms.Controls.Add(btnRefresh);

            btnJoin = new Button()
            {
                Text = "Join Room",
                Location = new Point(210, 185),
                Size = new Size(190, 35),
                BackColor = Color.FromArgb(0, 120, 200),
                ForeColor = Color.White,
                FlatStyle = FlatStyle.Flat,
                Cursor = Cursors.Hand,
                Enabled = false
            };
            btnJoin.Click += BtnJoin_Click;
            grpRooms.Controls.Add(btnJoin);

            this.Controls.Add(grpRooms);

            // Lobby Group (hidden initially)
            grpLobby = new GroupBox()
            {
                Text = "Room Lobby",
                Location = new Point(12, 12),
                Size = new Size(410, 280),
                ForeColor = Color.White,
                Visible = false
            };

            lblRoomName = new Label()
            {
                Location = new Point(10, 25),
                Size = new Size(390, 30),
                ForeColor = Color.Gold,
                Font = new Font("Segoe UI", 10, FontStyle.Bold),
                TextAlign = ContentAlignment.MiddleCenter
            };
            grpLobby.Controls.Add(lblRoomName);

            Label lblPlayersTitle = new Label()
            {
                Text = "Players in this room:",
                Location = new Point(10, 65),
                Size = new Size(390, 20),
                ForeColor = Color.White,
                Font = new Font("Segoe UI", 9, FontStyle.Bold)
            };
            grpLobby.Controls.Add(lblPlayersTitle);

            lstPlayers = new ListBox()
            {
                Location = new Point(10, 90),
                Size = new Size(390, 100),
                BackColor = Color.FromArgb(30, 30, 30),
                ForeColor = Color.White,
                Font = new Font("Segoe UI", 10)
            };
            grpLobby.Controls.Add(lstPlayers);

            lblPlayerCount = new Label()
            {
                Location = new Point(10, 200),
                Size = new Size(390, 20),
                ForeColor = Color.LightGray,
                TextAlign = ContentAlignment.MiddleCenter
            };
            grpLobby.Controls.Add(lblPlayerCount);

            btnLeave = new Button()
            {
                Text = "Leave Room",
                Location = new Point(10, 230),
                Size = new Size(390, 35),
                BackColor = Color.FromArgb(150, 50, 50),
                ForeColor = Color.White,
                FlatStyle = FlatStyle.Flat,
                Cursor = Cursors.Hand
            };
            btnLeave.Click += BtnLeave_Click;
            grpLobby.Controls.Add(btnLeave);

            this.Controls.Add(grpLobby);

            lblStatus = new Label()
            {
                Location = new Point(12, 300),
                Size = new Size(410, 40),
                ForeColor = Color.Salmon,
                Text = ""
            };
            this.Controls.Add(lblStatus);
        }

        private void StartAutoRefresh()
        {
            _refreshTimer = new System.Windows.Forms.Timer();
            _refreshTimer.Interval = 2000;
            _refreshTimer.Tick += (s, e) =>
            {
                if (_gameStarted) return;

                if (_currentRoomId > 0)
                {
                    RefreshPlayerList();
                }
                else
                {
                    LoadRooms();
                }
            };
            _refreshTimer.Start();
        }

        private void LoadRooms()
        {
            if (_currentRoomId > 0 || _gameStarted) return;

            try
            {
                var (code, doc) = _comm.SendAndReceive(Codes.GET_ROOMS_REQUEST);

                _roomDictionary.Clear();
                lstRooms.Items.Clear();

                // Try new format with roomIds and roomNames
                if (doc.RootElement.TryGetProperty("roomIds", out JsonElement roomIds) &&
                    doc.RootElement.TryGetProperty("roomNames", out JsonElement roomNames))
                {
                    var ids = roomIds.EnumerateArray();
                    var names = roomNames.EnumerateArray();

                    var idsList = new List<int>();
                    var namesList = new List<string>();

                    foreach (var id in ids)
                        idsList.Add(id.GetInt32());
                    foreach (var name in names)
                        namesList.Add(name.GetString());

                    for (int i = 0; i < idsList.Count && i < namesList.Count; i++)
                    {
                        int roomId = idsList[i];
                        string roomName = namesList[i];
                        _roomDictionary[roomId] = roomName;
                        lstRooms.Items.Add($"{roomId}. {roomName}");
                    }
                }
                // Fallback to old format (Rooms array only)
                else if (doc.RootElement.TryGetProperty("Rooms", out JsonElement rooms))
                {
                    int roomId = 1;
                    foreach (var room in rooms.EnumerateArray())
                    {
                        string roomName = room.GetString();
                        _roomDictionary[roomId] = roomName;
                        lstRooms.Items.Add($"{roomId}. {roomName}");
                        roomId++;
                    }
                }

                if (lstRooms.Items.Count == 0)
                {
                    lstRooms.Items.Add("(No rooms available)");
                    btnJoin.Enabled = false;
                }
                else
                {
                    btnJoin.Enabled = lstRooms.SelectedIndex >= 0;
                }

                lblStatus.Text = "";
            }
            catch (Exception ex)
            {
                lstRooms.Items.Clear();
                lstRooms.Items.Add("(Error loading rooms)");
                lblStatus.Text = "Error loading rooms: " + ex.Message;
            }
        }

        private void BtnJoin_Click(object sender, EventArgs e)
        {
            if (lstRooms.SelectedIndex < 0) return;

            string selectedItem = lstRooms.SelectedItem.ToString();
            if (selectedItem.Contains("No rooms") || selectedItem.Contains("Error")) return;

            // Extract room ID from the selected item (format: "ID. RoomName")
            int dotIndex = selectedItem.IndexOf('.');
            if (dotIndex < 0) return;

            string roomIdStr = selectedItem.Substring(0, dotIndex).Trim();
            if (!int.TryParse(roomIdStr, out int roomId)) return;

            if (!_roomDictionary.TryGetValue(roomId, out string roomName))
            {
                lblStatus.Text = "Error: Room not found";
                return;
            }

            try
            {
                var (code, doc) = _comm.SendAndReceive(Codes.JOIN_ROOM_REQUEST, new { roomId });

                // Check for status in response
                bool success = false;
                if (doc.RootElement.TryGetProperty("status", out JsonElement status))
                {
                    success = status.GetInt32() == 1;
                }

                if (success)
                {
                    _currentRoomId = roomId;
                    _currentRoomName = roomName;

                    // Switch to lobby view
                    grpRooms.Visible = false;
                    grpLobby.Visible = true;
                    lblRoomName.Text = $"Room: {roomName}";

                    // Immediately refresh player list
                    RefreshPlayerList();
                }
                else
                {
                    string error = "Failed to join room.";
                    if (doc.RootElement.TryGetProperty("message", out JsonElement msg))
                        error = msg.GetString();
                    lblStatus.Text = error;
                }
            }
            catch (Exception ex)
            {
                lblStatus.Text = "Error joining room: " + ex.Message;
            }
        }

        private void RefreshPlayerList()
        {
            if (_currentRoomId <= 0 || _gameStarted) return;

            try
            {
                // First check room state to see if game started
                var (stateCode, stateDoc) = _comm.SendAndReceive(Codes.GET_ROOM_STATE_REQUEST, new { });

                // Check if game has begun
                if (stateDoc.RootElement.TryGetProperty("hasGameBegun", out JsonElement hasGameBegun))
                {
                    if (hasGameBegun.GetBoolean())
                    {
                        _gameStarted = true;
                        _refreshTimer?.Stop();

                        int questionCount = 10;
                        int answerTimeout = 30;
                        if (stateDoc.RootElement.TryGetProperty("questionCount", out JsonElement qc))
                            questionCount = qc.GetInt32();
                        if (stateDoc.RootElement.TryGetProperty("answerTimeout", out JsonElement at))
                            answerTimeout = at.GetInt32();

                        this.Invoke(new Action(() =>
                        {
                            var gameForm = new GameForm(_comm, _username, _currentRoomId, answerTimeout, questionCount);
                            this.Hide();
                            gameForm.ShowDialog();
                            this.Close();
                        }));
                        return;
                    }
                }

                // Continue with normal player list refresh (only if game hasn't started)
                var (code, doc) = _comm.SendAndReceive(Codes.GET_PLAYERS_IN_ROOM_REQUEST, new { roomId = _currentRoomId });

                lstPlayers.Items.Clear();
                int playerCount = 0;

                // Check if room was closed or doesn't exist
                if (doc.RootElement.TryGetProperty("message", out JsonElement msg))
                {
                    string message = msg.GetString();
                    if (message.Contains("not found") || message.Contains("doesn't exist"))
                    {
                        // Room was closed/deleted - automatically leave
                        MessageBox.Show("The room has been closed by the admin.", "Room Closed",
                                       MessageBoxButtons.OK, MessageBoxIcon.Information);

                        // Clean up and go back to room list
                        _currentRoomId = -1;
                        _currentRoomName = "";
                        _gameStarted = false;
                        grpLobby.Visible = false;
                        grpRooms.Visible = true;
                        LoadRooms();
                        return;
                    }
                }

                // Try different possible JSON field names
                if (doc.RootElement.TryGetProperty("PlayersInRoom", out JsonElement players))
                {
                    foreach (var player in players.EnumerateArray())
                    {
                        string playerName = player.GetString();
                        playerCount++;
                        if (playerName == _username)
                            lstPlayers.Items.Add($"{playerName} (YOU)");
                        else
                            lstPlayers.Items.Add(playerName);
                    }
                }
                else if (doc.RootElement.TryGetProperty("players", out players))
                {
                    foreach (var player in players.EnumerateArray())
                    {
                        string playerName = player.GetString();
                        playerCount++;
                        if (playerName == _username)
                            lstPlayers.Items.Add($"{playerName} (YOU)");
                        else
                            lstPlayers.Items.Add(playerName);
                    }
                }

                lblPlayerCount.Text = $"Total players: {playerCount}";

                if (playerCount == 0 && lstPlayers.Items.Count == 0)
                {
                    lstPlayers.Items.Add("(Waiting for players...)");
                }
            }
            catch (Exception ex)
            {
                // If we get an exception, the room probably closed
                // Auto-leave and go back to room list
                _currentRoomId = -1;
                _currentRoomName = "";
                _gameStarted = false;
                grpLobby.Visible = false;
                grpRooms.Visible = true;
                LoadRooms();
            }
        }

        private void BtnLeave_Click(object sender, EventArgs e)
        {
            LeaveRoom();
        }

        private void LeaveRoom()
        {
            if (_currentRoomId > 0)
            {
                try
                {
                    _comm.SendAndReceive(Codes.LEAVE_ROOM_REQUEST, new { });
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Error leaving room: {ex.Message}");
                }
            }

            _currentRoomId = -1;
            _currentRoomName = "";
            _gameStarted = false;
            grpLobby.Visible = false;
            grpRooms.Visible = true;
            lstPlayers.Items.Clear();
            lblPlayerCount.Text = "Total players: 0";
            LoadRooms();
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            _refreshTimer?.Stop();
            _refreshTimer?.Dispose();

            if (_currentRoomId > 0 && !_gameStarted)
            {
                try
                {
                    _comm.SendAndReceive(Codes.LEAVE_ROOM_REQUEST, new { });
                }
                catch { }
            }

            base.OnFormClosing(e);
        }
    }
}