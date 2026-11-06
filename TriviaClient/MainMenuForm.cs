using System;
using System.Drawing;
using System.Windows.Forms;

namespace TriviaClient
{
    public class MainMenuForm : Form
    {
        private readonly Communicator _comm;
        private readonly string _username;
        private bool _signedOut = false;

        public MainMenuForm(Communicator comm, string username)
        {
            _comm = comm;
            _username = username;

            Text = $"Trivia Client - Main Menu ({username})";
            Size = new Size(320, 380);
            StartPosition = FormStartPosition.CenterScreen;
            FormBorderStyle = FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            BackColor = Color.FromArgb(45, 45, 48);
            ForeColor = Color.WhiteSmoke;
            Font = new Font("Segoe UI", 10f);

            var lblWelcome = new Label
            {
                Text = $"Welcome, {username}!",
                Location = new Point(20, 16),
                AutoSize = true,
                ForeColor = Color.LightSkyBlue,
                Font = new Font("Segoe UI", 11f, FontStyle.Bold)
            };
            Controls.Add(lblWelcome);

            (string label, Color color, Action action)[] items = {
                ("Create Room",    Color.FromArgb(0, 122, 204),  OpenCreateRoom),
                ("Join Room",      Color.FromArgb(0, 122, 204),  OpenJoinRoom),
                ("My Statistics",  Color.FromArgb(100, 140, 60), OpenStats),
                ("High Scores",    Color.FromArgb(100, 140, 60), OpenHighScores),
                ("Sign Out",       Color.FromArgb(160, 60, 60),  DoSignout),
                ("Shalom",         Color.FromArgb(160, 60, 60),  DoSignout)
            };

            int y = 52;
            foreach (var (label, color, action) in items)
            {
                var btn = new Button
                {
                    Text = label,
                    Location = new Point(20, y),
                    Size = new Size(264, 38),
                    BackColor = color,
                    ForeColor = Color.White,
                    FlatStyle = FlatStyle.Flat,
                    Font = new Font("Segoe UI", 10f),
                    Cursor = Cursors.Hand
                };
                btn.FlatAppearance.BorderSize = 0;
                var act = action;
                btn.Click += (s, e) => act();
                Controls.Add(btn);
                y += 48;
            }

            // Send signout if user closes via X instead of the Sign Out button
            this.FormClosing += (s, e) =>
            {
                if (!_signedOut)
                {
                    try { _comm.SendAndReceive(Codes.SIGNOUT_REQUEST); } catch { }
                }
            };
        }

        private void OpenCreateRoom()
        {
            new CreateRoomForm(_comm, _username).ShowDialog(this);
        }

        private void OpenJoinRoom()
        {
            new JoinRoomForm(_comm, _username).ShowDialog(this);
        }

        private void OpenStats()
        {
            new StatisticsForm(_comm, _username, highScoreMode: false).ShowDialog(this);
        }

        private void OpenHighScores()
        {
            new StatisticsForm(_comm, _username, highScoreMode: true).ShowDialog(this);
        }

        private void DoSignout()
        {
            if (MessageBox.Show("Sign out?", "Confirm", MessageBoxButtons.YesNo) != DialogResult.Yes)
                return;
            try { _comm.SendAndReceive(Codes.SIGNOUT_REQUEST); } catch { }
            _signedOut = true; // prevent FormClosing from sending it again
            DialogResult = DialogResult.Retry;
            Close();
        }
        
        private void DoShalom()
        {

        }
    }
}