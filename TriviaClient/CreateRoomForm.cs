using System;
using System.Drawing;
using System.Text.Json;
using System.Windows.Forms;

namespace TriviaClient
{
    public class CreateRoomForm : Form
    {
        private readonly Communicator _comm;
        private readonly string _username;
        private int _currentRoomId = -1;
        private bool _gameStarted = false;
        private bool _isCreator = true; // Creator is always the one who created the room

        private TextBox txtRoomName, txtMaxPlayers, txtTimePerQuestion;
        private Button btnCreate, btnClose, btnStartGame;
        private Label lblStatus, lblRoomInfo, lblPlayerCount;
        private GroupBox grpSetup, grpLobby;
        private ListBox lstPlayers;
        private System.Windows.Forms.Timer _refreshTimer;

        public CreateRoomForm(Communicator comm, string username)
        {
            _comm = comm;
            _username = username;
            InitializeComponents();
        }

        private void InitializeComponents()
        {
            this.Text = "Trivia Client - Create Room";
            this.Size = new Size(450, 600);
            this.StartPosition = FormStartPosition.CenterParent;
            this.FormBorderStyle = FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.BackColor = Color.FromArgb(45, 45, 48);

            // Setup Group
            grpSetup = new GroupBox()
            {
                Text = "Room Setup",
                Location = new Point(12, 12),
                Size = new Size(410, 200),
                ForeColor = Color.White
            };

            Label lblName = new Label()
            {
                Text = "Room Name:",
                Location = new Point(10, 30),
                Size = new Size(100, 25),
                ForeColor = Color.White
            };
            grpSetup.Controls.Add(lblName);

            txtRoomName = new TextBox()
            {
                Location = new Point(120, 28),
                Size = new Size(270, 25),
                BackColor = Color.FromArgb(30, 30, 30),
                ForeColor = Color.White,
                Text = "My Game Room"
            };
            grpSetup.Controls.Add(txtRoomName);

            Label lblMax = new Label()
            {
                Text = "Max Players (2-8):",
                Location = new Point(10, 65),
                Size = new Size(100, 25),
                ForeColor = Color.White
            };
            grpSetup.Controls.Add(lblMax);

            txtMaxPlayers = new TextBox()
            {
                Location = new Point(120, 63),
                Size = new Size(80, 25),
                BackColor = Color.FromArgb(30, 30, 30),
                ForeColor = Color.White,
                Text = "4"
            };
            grpSetup.Controls.Add(txtMaxPlayers);

            Label lblTime = new Label()
            {
                Text = "Seconds/Question:",
                Location = new Point(10, 100),
                Size = new Size(100, 25),
                ForeColor = Color.White
            };
            grpSetup.Controls.Add(lblTime);

            txtTimePerQuestion = new TextBox()
            {
                Location = new Point(120, 98),
                Size = new Size(80, 25),
                BackColor = Color.FromArgb(30, 30, 30),
                ForeColor = Color.White,
                Text = "30"
            };
            grpSetup.Controls.Add(txtTimePerQuestion);

            btnCreate = new Button()
            {
                Text = "Create Room",
                Location = new Point(10, 145),
                Size = new Size(390, 40),
                BackColor = Color.FromArgb(0, 120, 200),
                ForeColor = Color.White,
                FlatStyle = FlatStyle.Flat,
                Font = new Font("Segoe UI", 10, FontStyle.Bold),
                Cursor = Cursors.Hand
            };
            btnCreate.Click += BtnCreate_Click;
            grpSetup.Controls.Add(btnCreate);

            this.Controls.Add(grpSetup);

            // Lobby Group (hidden initially)
            grpLobby = new GroupBox()
            {
                Text = "Room Lobby",
                Location = new Point(12, 12),
                Size = new Size(410, 330),
                ForeColor = Color.White,
                Visible = false
            };

            lblRoomInfo = new Label()
            {
                Location = new Point(10, 25),
                Size = new Size(390, 30),
                ForeColor = Color.Gold,
                Font = new Font("Segoe UI", 10, FontStyle.Bold),
                TextAlign = ContentAlignment.MiddleCenter
            };
            grpLobby.Controls.Add(lblRoomInfo);

            Label lblPlayersTitle = new Label()
            {
                Text = "Players in room:",
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

            // Start Game button (Admin only)
            btnStartGame = new Button()
            {
                Text = "Start Game",
                Location = new Point(10, 230),
                Size = new Size(390, 40),
                BackColor = Color.FromArgb(0, 150, 50),
                ForeColor = Color.White,
                FlatStyle = FlatStyle.Flat,
                Font = new Font("Segoe UI", 10, FontStyle.Bold),
                Cursor = Cursors.Hand
            };
            btnStartGame.Click += BtnStartGame_Click;
            grpLobby.Controls.Add(btnStartGame);

            btnClose = new Button()
            {
                Text = "Close Room",
                Location = new Point(10, 280),
                Size = new Size(390, 35),
                BackColor = Color.FromArgb(150, 50, 50),
                ForeColor = Color.White,
                FlatStyle = FlatStyle.Flat,
                Font = new Font("Segoe UI", 9, FontStyle.Bold),
                Cursor = Cursors.Hand
            };
            btnClose.Click += BtnClose_Click;
            grpLobby.Controls.Add(btnClose);

            this.Controls.Add(grpLobby);

            lblStatus = new Label()
            {
                Location = new Point(12, 220),
                Size = new Size(410, 40),
                ForeColor = Color.Salmon,
                Text = ""
            };
            this.Controls.Add(lblStatus);
        }

        private void BtnCreate_Click(object sender, EventArgs e)
        {
            lblStatus.Text = "";

            if (!int.TryParse(txtMaxPlayers.Text, out int maxPlayers) || maxPlayers < 2 || maxPlayers > 8)
            {
                lblStatus.Text = "Max players must be between 2 and 8";
                return;
            }

            if (!int.TryParse(txtTimePerQuestion.Text, out int timePerQuestion) || timePerQuestion < 5 || timePerQuestion > 120)
            {
                lblStatus.Text = "Time per question must be between 5 and 120 seconds";
                return;
            }

            if (string.IsNullOrWhiteSpace(txtRoomName.Text))
            {
                lblStatus.Text = "Room name cannot be empty";
                return;
            }

            try
            {
                var (code, doc) = _comm.SendAndReceive(Codes.CREATE_ROOM_REQUEST, new
                {
                    roomName = txtRoomName.Text.Trim(),
                    maxPlayers = maxPlayers,
                    timePerQuestion = timePerQuestion
                });

                // Get room ID from response
                if (doc.RootElement.TryGetProperty("roomId", out JsonElement roomIdElem))
                {
                    _currentRoomId = roomIdElem.GetInt32();
                }

                if (doc.RootElement.TryGetProperty("status", out JsonElement status) && status.GetInt32() == 1 && _currentRoomId > 0)
                {
                    // Switch to lobby view
                    grpSetup.Visible = false;
                    grpLobby.Visible = true;
                    lblRoomInfo.Text = $"Room: {txtRoomName.Text.Trim()} (You are the Admin)";
                    btnStartGame.Visible = true; // Admin can start game

                    // Start refreshing player list
                    StartRefreshingPlayers();

                    // Initial player list update
                    RefreshPlayerList();
                }
                else
                {
                    string error = "Failed to create room.";
                    if (doc.RootElement.TryGetProperty("message", out JsonElement msg))
                        error = msg.GetString();
                    lblStatus.Text = error;
                }
            }
            catch (Exception ex)
            {
                lblStatus.Text = "Error: " + ex.Message;
            }
        }

        private void BtnStartGame_Click(object sender, EventArgs e)
        {
            if (_currentRoomId <= 0 || _gameStarted) return;

            try
            {
                lblStatus.Text = "Starting game...";
                var (code, doc) = _comm.SendAndReceive(Codes.START_GAME_REQUEST, new { });

                if (doc.RootElement.TryGetProperty("status", out JsonElement status) && status.GetInt32() == 1)
                {
                    // Game started successfully - the RefreshPlayerList will detect hasGameBegun
                    // and launch the game form automatically
                    lblStatus.Text = "Game starting...";
                    btnStartGame.Enabled = false;
                }
                else
                {
                    string error = "Failed to start game.";
                    if (doc.RootElement.TryGetProperty("message", out JsonElement msg))
                        error = msg.GetString();
                    lblStatus.Text = error;
                }
            }
            catch (Exception ex)
            {
                lblStatus.Text = "Error starting game: " + ex.Message;
            }
        }

        private void BtnClose_Click(object sender, EventArgs e)
        {
            if (_currentRoomId > 0 && !_gameStarted)
            {
                try
                {
                    _comm.SendAndReceive(Codes.CLOSE_ROOM_REQUEST, new { });
                    _currentRoomId = -1;
                }
                catch (Exception ex)
                {
                    MessageBox.Show($"Error closing room: {ex.Message}", "Error",
                                MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }

            this.Close();
        }

        private void StartRefreshingPlayers()
        {
            _refreshTimer = new System.Windows.Forms.Timer();
            _refreshTimer.Interval = 2000;
            _refreshTimer.Tick += (s, e) => RefreshPlayerList();
            _refreshTimer.Start();
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

                        // Get game details
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

                // Continue with normal player list refresh
                var (code, doc) = _comm.SendAndReceive(Codes.GET_PLAYERS_IN_ROOM_REQUEST, new { roomId = _currentRoomId });

                lstPlayers.Items.Clear();
                int playerCount = 0;

                // Check if room was closed
                if (doc.RootElement.TryGetProperty("message", out JsonElement msg))
                {
                    string message = msg.GetString();
                    if (message.Contains("not found") || message.Contains("doesn't exist"))
                    {
                        MessageBox.Show("Your room has been closed.", "Room Closed",
                                       MessageBoxButtons.OK, MessageBoxIcon.Information);
                        this.Close();
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
                            lstPlayers.Items.Add($"{playerName} (Admin)");
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
                            lstPlayers.Items.Add($"{playerName} (Admin)");
                        else
                            lstPlayers.Items.Add(playerName);
                    }
                }

                lblPlayerCount.Text = $"Total players: {playerCount}";
                btnStartGame.Enabled = playerCount >= 2 && !_gameStarted;

                if (playerCount == 0 && lstPlayers.Items.Count == 0)
                {
                    lstPlayers.Items.Add("(No players in room)");
                }
            }
            catch (Exception ex)
            {
                this.Close();
            }
        }

        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            _refreshTimer?.Stop();
            _refreshTimer?.Dispose();

            if (_currentRoomId > 0 && !_gameStarted)
            {
                try
                {
                    _comm.SendAndReceive(Codes.CLOSE_ROOM_REQUEST, new { });
                    _currentRoomId = -1;
                }
                catch { }
            }

            base.OnFormClosed(e);
        }
    }
}